#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
FLATPAK_MANIFEST="${SOURCE_ROOT}/packaging/flatpak/io.github.guedessoftware.PurrView.yml"
FLATHUB_TEMPLATE="${SOURCE_ROOT}/packaging/flatpak/io.github.guedessoftware.PurrView.flathub.yml.in"

if [[ ! "${VERSION_VALUE}" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?(\+[0-9A-Za-z.-]+)?$ ]]; then
    printf 'Invalid VERSION: %s\n' "${VERSION_VALUE}" >&2
    exit 1
fi

grep -Fq "## [${VERSION_VALUE}]" "${SOURCE_ROOT}/CHANGELOG.md"
test -f "${SOURCE_ROOT}/docs/releases/${VERSION_VALUE}.md"
grep -Fq '@PURRVIEW_VERSION@' \
    "${SOURCE_ROOT}/resources/linux/io.github.guedessoftware.PurrView.metainfo.xml.in"
grep -Fq 'qt_add_executable(purrview' "${SOURCE_ROOT}/CMakeLists.txt"
grep -Fq 'exec "$(dirname "$0")/purrview" "$@"' \
    "${SOURCE_ROOT}/resources/bin/impage"
grep -Fq 'exec "$(dirname "$0")/install-purrview.sh" "$@"' \
    "${SOURCE_ROOT}/scripts/install-impage.sh"

LEGACY_PUBLIC_FILES=(
    "${SOURCE_ROOT}/resources/linux/io.github.impage.Impage.desktop"
    "${SOURCE_ROOT}/resources/linux/io.github.impage.Impage.Viewer.desktop"
    "${SOURCE_ROOT}/resources/linux/io.github.impage.Impage.metainfo.xml.in"
    "${SOURCE_ROOT}/resources/linux/impage-servicemenu.desktop"
    "${SOURCE_ROOT}/packaging/flatpak/io.github.impage.Impage.yml"
)
for legacy_file in "${LEGACY_PUBLIC_FILES[@]}"; do
    test ! -e "${legacy_file}"
done
if grep -R -Fq 'io.github.impage.Impage' \
    "${SOURCE_ROOT}/cmake" "${SOURCE_ROOT}/packaging/arch" \
    "${SOURCE_ROOT}/packaging/containers" "${FLATPAK_MANIFEST}" "${FLATHUB_TEMPLATE}" \
    "${SOURCE_ROOT}/qml" \
    "${SOURCE_ROOT}/resources" "${SOURCE_ROOT}/src" \
    "${SOURCE_ROOT}/CMakeLists.txt"; then
    printf 'Legacy App ID remains in a public or runtime file.\n' >&2
    exit 1
fi

SCREENSHOT_COUNT="$(grep -c '    <screenshot' \
    "${SOURCE_ROOT}/resources/linux/io.github.guedessoftware.PurrView.metainfo.xml.in")"
[[ "${SCREENSHOT_COUNT}" -ge 4 ]]

lock_value() {
    sed -n "s/^set($1 \"\\([^\"]*\\)\")$/\\1/p" \
        "${SOURCE_ROOT}/cmake/DependencyVersions.cmake"
}

grep -Fq "runtime-version: '$(lock_value PURRVIEW_FLATPAK_RUNTIME_VERSION)'" "${FLATPAK_MANIFEST}"
grep -Fq "/v$(lock_value PURRVIEW_EXIV2_FALLBACK_VERSION).tar.gz" "${FLATPAK_MANIFEST}"
grep -Fq "sha256: $(lock_value PURRVIEW_EXIV2_FALLBACK_SHA256)" "${FLATPAK_MANIFEST}"
grep -Fq 'url: https://github.com/guedessoftware/purrview/releases/download/v@VERSION@/' \
    "${FLATHUB_TEMPLATE}"
grep -Fq 'sha256: @SOURCE_SHA256@' "${FLATHUB_TEMPLATE}"

PACKAGING_FILES=(
    "${SOURCE_ROOT}/cmake/Packaging.cmake"
    "${SOURCE_ROOT}/packaging/arch/PKGBUILD.in"
    "${SOURCE_ROOT}/packaging/containers/deb.Dockerfile"
    "${SOURCE_ROOT}/packaging/containers/rpm.Dockerfile"
    "${SOURCE_ROOT}/packaging/containers/arch.Dockerfile"
)
for packaging_file in "${PACKAGING_FILES[@]}"; do
    test -s "${packaging_file}"
done

PACKAGING_SCRIPTS=(
    "${SOURCE_ROOT}/scripts/package-all.sh"
    "${SOURCE_ROOT}/packaging/containers/build-deb.sh"
    "${SOURCE_ROOT}/packaging/containers/build-rpm.sh"
    "${SOURCE_ROOT}/packaging/containers/build-arch.sh"
    "${SOURCE_ROOT}/packaging/containers/build-flatpak.sh"
    "${SOURCE_ROOT}/scripts/install-purrview.sh"
    "${SOURCE_ROOT}/scripts/install-impage.sh"
    "${SOURCE_ROOT}/scripts/sign-checksums.sh"
)
for packaging_script in "${PACKAGING_SCRIPTS[@]}"; do
    test -x "${packaging_script}"
    bash -n "${packaging_script}"
done

grep -Fq 'set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)' "${SOURCE_ROOT}/cmake/Packaging.cmake"
grep -Fq 'set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)' "${SOURCE_ROOT}/cmake/Packaging.cmake"
grep -Fq 'sha256sums=('\''@SOURCE_SHA256@'\'')' "${SOURCE_ROOT}/packaging/arch/PKGBUILD.in"

if command -v desktop-file-validate >/dev/null 2>&1; then
    desktop-file-validate "${SOURCE_ROOT}/resources/linux/io.github.guedessoftware.PurrView.desktop"
    desktop-file-validate "${SOURCE_ROOT}/resources/linux/io.github.guedessoftware.PurrView.Viewer.desktop"
fi

if (($# > 0)); then
    GENERATED_METAINFO="$1"
    grep -Fq "version=\"${VERSION_VALUE}\"" "${GENERATED_METAINFO}"
    if command -v appstreamcli >/dev/null 2>&1; then
        appstreamcli validate --no-net "${GENERATED_METAINFO}"
    fi
fi

printf 'Release metadata is consistent for PurrView %s.\n' "${VERSION_VALUE}"
