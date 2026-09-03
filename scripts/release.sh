#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
OUTPUT_DIR="${SOURCE_ROOT}/dist/${VERSION_VALUE}"
SKIP_BUILD=0
REQUIRE_FLATPAK=0

usage() {
    cat <<'EOF'
Usage: scripts/release.sh [--skip-build] [--require-flatpak] [--output DIR]

Builds/tests the release, creates reproducible source/bootstrap archives and SHA256SUMS.
It never creates tags, pushes commits or publishes artifacts.
EOF
}

while (($# > 0)); do
    case "$1" in
    --skip-build) SKIP_BUILD=1 ;;
    --require-flatpak) REQUIRE_FLATPAK=1 ;;
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
    cmake --preset release
    cmake --build --preset release
    ctest --preset release
    cmake --build "${SOURCE_ROOT}/build/release" --target impage_qmllint
fi

GENERATED_METAINFO="${SOURCE_ROOT}/build/release/generated/io.github.impage.Impage.metainfo.xml"
[[ -f "${GENERATED_METAINFO}" ]] || {
    printf 'Missing generated AppStream metadata; run the Release preset first.\n' >&2
    exit 1
}
"${SCRIPT_DIR}/validate-release.sh" "${GENERATED_METAINFO}"
[[ "$("${SOURCE_ROOT}/build/release/impage" --version)" == "PurrView ${VERSION_VALUE}" ]]

mkdir -p "${OUTPUT_DIR}"
SOURCE_ARCHIVE="PurrView-${VERSION_VALUE}-source.tar.xz"
BOOTSTRAP_ARCHIVE="PurrView-${VERSION_VALUE}-bootstrap.tar.xz"
rm -f -- "${OUTPUT_DIR}/${SOURCE_ARCHIVE}" "${OUTPUT_DIR}/${BOOTSTRAP_ARCHIVE}" \
    "${OUTPUT_DIR}/PurrView-${VERSION_VALUE}-x86_64.flatpak" "${OUTPUT_DIR}/SHA256SUMS"

if [[ -z "${SOURCE_DATE_EPOCH:-}" ]]; then
    RELEASE_DATE="$(sed -n 's/^set(IMPAGE_RELEASE_DATE "\([^"]*\)")$/\1/p' \
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
    THIRD_PARTY_NOTICES.md cmake docs packaging qml resources scripts src tests)
tar "${COMMON_TAR_ARGS[@]}" --transform "s,^,PurrView-${VERSION_VALUE}-bootstrap/," \
    -cJf "${OUTPUT_DIR}/${BOOTSTRAP_ARCHIVE}" -C "${SOURCE_ROOT}" "${BOOTSTRAP_INPUTS[@]}"

FLATPAK_NAME="PurrView-${VERSION_VALUE}-x86_64.flatpak"
if command -v flatpak-builder >/dev/null 2>&1 && \
    flatpak info org.kde.Sdk//6.10 >/dev/null 2>&1; then
    flatpak-builder --force-clean --repo="${OUTPUT_DIR}/flatpak-repo" \
        "${SOURCE_ROOT}/.flatpak-builder/release" \
        "${SOURCE_ROOT}/packaging/flatpak/io.github.impage.Impage.yml"
    flatpak build-bundle "${OUTPUT_DIR}/flatpak-repo" "${OUTPUT_DIR}/${FLATPAK_NAME}" \
        io.github.impage.Impage
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

printf 'Release artifacts prepared in %s\n' "${OUTPUT_DIR}"
printf 'No Git tag or remote publication was performed.\n'
