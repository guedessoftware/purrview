#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
OUTPUT_DIR="${SOURCE_ROOT}/dist/${VERSION_VALUE}"
SKIP_BUILD=0
REQUIRE_FLATPAK=0
SKIP_FLATPAK=0
SIGN_CHECKSUMS=0
GPG_KEY="${PURRVIEW_GPG_KEY:-}"

usage() {
    cat <<'EOF'
Usage: scripts/release.sh [options]

Builds/tests the release, creates reproducible source/bootstrap archives and SHA256SUMS.
It never creates tags, pushes commits or publishes artifacts.

  --skip-build          Reuse the existing validated Release build
  --require-flatpak     Fail if the Flatpak toolchain is unavailable
  --skip-flatpak        Prepare only source/bootstrap artifacts
  --sign-checksums      Create SHA256SUMS.asc with the default GPG key
  --gpg-key KEY_ID      Sign with KEY_ID (also enables --sign-checksums)
  --output DIR          Artifact directory (default: dist/VERSION)
EOF
}

while (($# > 0)); do
    case "$1" in
    --skip-build) SKIP_BUILD=1 ;;
    --require-flatpak) REQUIRE_FLATPAK=1 ;;
    --skip-flatpak) SKIP_FLATPAK=1 ;;
    --sign-checksums) SIGN_CHECKSUMS=1 ;;
    --gpg-key)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        GPG_KEY="$1"
        SIGN_CHECKSUMS=1
        ;;
    --output)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        OUTPUT_DIR="$1"
        ;;
    --help | -h) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
    esac
    shift
done

"${SCRIPT_DIR}/validate-release.sh"

if git -C "${SOURCE_ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    if ! git -C "${SOURCE_ROOT}" diff --quiet || \
        ! git -C "${SOURCE_ROOT}" diff --cached --quiet || \
        [[ -n "$(git -C "${SOURCE_ROOT}" ls-files --others --exclude-standard)" ]]; then
        printf 'Release requires a clean Git working tree.\n' >&2
        exit 1
    fi
else
    printf '! No Git repository found; tag and commit validation were skipped.\n' >&2
fi

if ((!SKIP_BUILD)); then
    (
        cd "${SOURCE_ROOT}"
        cmake --preset release
        cmake --build --preset release
        ctest --preset release
    )
    cmake --build "${SOURCE_ROOT}/build/release" --target purrview_qmllint
fi

GENERATED_METAINFO="${SOURCE_ROOT}/build/release/generated/io.github.guedessoftware.PurrView.metainfo.xml"
[[ -f "${GENERATED_METAINFO}" ]] || {
    printf 'Missing generated AppStream metadata; run the Release preset first.\n' >&2
    exit 1
}
"${SCRIPT_DIR}/validate-release.sh" "${GENERATED_METAINFO}"
[[ "$("${SOURCE_ROOT}/build/release/purrview" --version)" == "PurrView ${VERSION_VALUE}" ]]

mkdir -p "${OUTPUT_DIR}"
SOURCE_ARCHIVE="PurrView-${VERSION_VALUE}-source.tar.xz"
BOOTSTRAP_ARCHIVE="PurrView-${VERSION_VALUE}-bootstrap.tar.xz"
rm -f -- "${OUTPUT_DIR}/${SOURCE_ARCHIVE}" "${OUTPUT_DIR}/${BOOTSTRAP_ARCHIVE}" \
    "${OUTPUT_DIR}/PurrView-${VERSION_VALUE}-x86_64.flatpak" \
    "${OUTPUT_DIR}/SHA256SUMS" "${OUTPUT_DIR}/SHA256SUMS.asc"

if [[ -z "${SOURCE_DATE_EPOCH:-}" ]]; then
    RELEASE_DATE="$(sed -n 's/^set(PURRVIEW_RELEASE_DATE "\([^"]*\)")$/\1/p' \
        "${SOURCE_ROOT}/cmake/ReleaseMetadata.cmake")"
    SOURCE_DATE_EPOCH="$(date -u -d "${RELEASE_DATE} 00:00:00" +%s)"
fi
export SOURCE_DATE_EPOCH XZ_OPT='-T1 -9e'

COMMON_TAR_ARGS=(--sort=name --mtime="@${SOURCE_DATE_EPOCH}" --owner=0 --group=0
    --numeric-owner --exclude='./.git' --exclude='./build' --exclude='./build-*'
    --exclude='./CMakeFiles' --exclude='./.cache' --exclude='./.flatpak-builder'
    --exclude='./dist' --exclude='*.user')

tar "${COMMON_TAR_ARGS[@]}" --transform "s,^\.,PurrView-${VERSION_VALUE}," \
    -cJf "${OUTPUT_DIR}/${SOURCE_ARCHIVE}" -C "${SOURCE_ROOT}" .

BOOTSTRAP_INPUTS=(CMakeLists.txt CMakePresets.json VERSION LICENSE README.md CHANGELOG.md
    THIRD_PARTY_NOTICES.md PurrView_Development_Guide.md cmake docs packaging qml resources
    scripts src tests)
tar "${COMMON_TAR_ARGS[@]}" --transform "s,^,PurrView-${VERSION_VALUE}-bootstrap/," \
    -cJf "${OUTPUT_DIR}/${BOOTSTRAP_ARCHIVE}" -C "${SOURCE_ROOT}" "${BOOTSTRAP_INPUTS[@]}"

FLATPAK_NAME="PurrView-${VERSION_VALUE}-x86_64.flatpak"
if ((SKIP_FLATPAK)); then
    printf '! Flatpak artifact skipped by request.\n' >&2
elif command -v flatpak-builder >/dev/null 2>&1 && \
    flatpak info org.kde.Sdk//6.10 >/dev/null 2>&1; then
    flatpak-builder --force-clean --repo="${OUTPUT_DIR}/flatpak-repo" \
        "${SOURCE_ROOT}/.flatpak-builder/release" \
        "${SOURCE_ROOT}/packaging/flatpak/io.github.guedessoftware.PurrView.yml"
    flatpak build-bundle "${OUTPUT_DIR}/flatpak-repo" "${OUTPUT_DIR}/${FLATPAK_NAME}" \
        io.github.guedessoftware.PurrView
elif ((REQUIRE_FLATPAK)); then
    printf 'Flatpak builder or org.kde.Sdk//6.10 is unavailable.\n' >&2
    exit 1
else
    printf '! Flatpak artifact skipped: builder/runtime unavailable.\n' >&2
fi

ARTIFACTS=("${SOURCE_ARCHIVE}" "${BOOTSTRAP_ARCHIVE}")
[[ ! -f "${OUTPUT_DIR}/${FLATPAK_NAME}" ]] || ARTIFACTS+=("${FLATPAK_NAME}")
(
    cd "${OUTPUT_DIR}"
    sha256sum "${ARTIFACTS[@]}" >SHA256SUMS
)
if ((SIGN_CHECKSUMS)); then
    signing_arguments=("${OUTPUT_DIR}/SHA256SUMS")
    [[ -z "${GPG_KEY}" ]] || signing_arguments+=(--key "${GPG_KEY}")
    "${SCRIPT_DIR}/sign-checksums.sh" "${signing_arguments[@]}"
fi

printf 'Release artifacts prepared in %s\n' "${OUTPUT_DIR}"
printf 'No Git tag or remote publication was performed.\n'
