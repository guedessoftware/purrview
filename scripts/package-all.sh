#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
OUTPUT_DIR="${SOURCE_ROOT}/dist/${VERSION_VALUE}"
CONTAINER_ENGINE="${CONTAINER_ENGINE:-docker}"
SKIP_HOST_BUILD=0
REFRESH_IMAGES=0
FORMATS=(deb rpm arch flatpak)

usage() {
    cat <<'EOF'
Usage: scripts/package-all.sh [options]

Creates the source/bootstrap archives, then compiles and validates every binary
package inside its target environment.

Options:
  --formats LIST       Comma-separated: deb,rpm,arch,flatpak (default: all)
  --output DIR         Artifact directory (default: dist/VERSION)
  --skip-host-build    Reuse the existing validated Release build
  --refresh-images     Pull newer revisions of the pinned container tags
  --engine COMMAND     OCI engine (default: docker or CONTAINER_ENGINE)
  -h, --help           Show this help
EOF
}

while (($# > 0)); do
    case "$1" in
    --formats)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        IFS=',' read -r -a FORMATS <<<"$1"
        ;;
    --output)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        OUTPUT_DIR="$1"
        ;;
    --skip-host-build) SKIP_HOST_BUILD=1 ;;
    --refresh-images) REFRESH_IMAGES=1 ;;
    --engine)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        CONTAINER_ENGINE="$1"
        ;;
    --help | -h) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
    esac
    shift
done

command -v "${CONTAINER_ENGINE}" >/dev/null 2>&1 || {
    printf 'Container engine not found: %s\n' "${CONTAINER_ENGINE}" >&2
    exit 1
}
"${CONTAINER_ENGINE}" info >/dev/null

declare -A REQUESTED=()
for format in "${FORMATS[@]}"; do
    case "${format}" in
    deb | rpm | arch | flatpak) REQUESTED["${format}"]=1 ;;
    *) printf 'Unknown package format: %s\n' "${format}" >&2; exit 2 ;;
    esac
done

release_arguments=(--output "${OUTPUT_DIR}")
((SKIP_HOST_BUILD == 0)) || release_arguments+=(--skip-build)
"${SCRIPT_DIR}/release.sh" "${release_arguments[@]}"

mkdir -p "${OUTPUT_DIR}"
rm -f -- "${OUTPUT_DIR}"/purrview_"${VERSION_VALUE}"_*.deb \
    "${OUTPUT_DIR}"/purrview-"${VERSION_VALUE}"-*.rpm \
    "${OUTPUT_DIR}"/purrview-"${VERSION_VALUE}"-*.pkg.tar.zst

docker_build() {
    local format="$1"
    local tag="purrview-builder-${format}:${VERSION_VALUE}"
    local pull_arguments=()
    ((REFRESH_IMAGES == 0)) || pull_arguments+=(--pull)
    "${CONTAINER_ENGINE}" build "${pull_arguments[@]}" \
        --file "${SOURCE_ROOT}/packaging/containers/${format}.Dockerfile" \
        --tag "${tag}" \
        "${SOURCE_ROOT}/packaging/containers"
    printf '%s\n' "${tag}"
}

run_native_builder() {
    local format="$1"
    local image
    image="$(docker_build "${format}" | tail -n 1)"
    "${CONTAINER_ENGINE}" run --rm \
        --volume "${SOURCE_ROOT}:/src:ro" \
        --volume "${OUTPUT_DIR}:/out" \
        "${image}"
}

for format in deb rpm arch; do
    [[ -n "${REQUESTED[${format}]:-}" ]] || continue
    printf '\n==> Building %s package in its stable container\n' "${format}"
    run_native_builder "${format}"
done

if [[ -n "${REQUESTED[flatpak]:-}" ]]; then
    FLATPAK_BUNDLE="${OUTPUT_DIR}/PurrView-${VERSION_VALUE}-x86_64.flatpak"
    if [[ ! -f "${FLATPAK_BUNDLE}" ]]; then
        FLATPAK_IMAGE="ghcr.io/flathub-infra/flatpak-github-actions:kde-6.8"
        if ((REFRESH_IMAGES)); then
            "${CONTAINER_ENGINE}" pull "${FLATPAK_IMAGE}"
        fi
        # Keep dependency state between releases. The Flatpak runtime version is
        # the compatibility boundary; changing the application version alone
        # must not rebuild third-party modules unnecessarily.
        HOST_CACHE_ROOT="${XDG_CACHE_HOME:-${HOME}/.cache}"
        FLATPAK_WORK="${HOST_CACHE_ROOT}/purrview/packaging/flatpak-kde-6.8"
        mkdir -p "${FLATPAK_WORK}"
        printf '\n==> Building Flatpak with the KDE 6.8 SDK container\n'
        "${CONTAINER_ENGINE}" run --rm --privileged \
            --entrypoint /bin/bash \
            --volume "${SOURCE_ROOT}:/src:ro" \
            --volume "${OUTPUT_DIR}:/out" \
            --volume "${FLATPAK_WORK}:/work" \
            "${FLATPAK_IMAGE}" \
            /src/packaging/containers/build-flatpak.sh
    else
        printf 'Reusing Flatpak already validated by release.sh: %s\n' "${FLATPAK_BUNDLE}"
    fi
fi

expected=(
    "PurrView-${VERSION_VALUE}-source.tar.xz"
    "PurrView-${VERSION_VALUE}-bootstrap.tar.xz"
)
[[ -z "${REQUESTED[deb]:-}" ]] || expected+=("purrview_${VERSION_VALUE}_amd64.deb")
[[ -z "${REQUESTED[rpm]:-}" ]] || expected+=("purrview-${VERSION_VALUE}-1.x86_64.rpm")
[[ -z "${REQUESTED[arch]:-}" ]] || expected+=("purrview-${VERSION_VALUE}-1-x86_64.pkg.tar.zst")
[[ -z "${REQUESTED[flatpak]:-}" ]] || expected+=("PurrView-${VERSION_VALUE}-x86_64.flatpak")

for artifact in "${expected[@]}"; do
    [[ -s "${OUTPUT_DIR}/${artifact}" ]] || {
        printf 'Expected artifact was not generated: %s\n' "${artifact}" >&2
        exit 1
    }
done

(
    cd "${OUTPUT_DIR}"
    printf '%s\n' "${expected[@]}" | LC_ALL=C sort | xargs sha256sum >SHA256SUMS
    sha256sum -c SHA256SUMS
)

printf '\nAll requested PurrView %s packages are ready in %s\n' \
    "${VERSION_VALUE}" "${OUTPUT_DIR}"
