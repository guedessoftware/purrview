#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE_ROOT=/src
WORK_ROOT=/tmp/purrview-build-arch
OUTPUT_ROOT=/out
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
SOURCE_ARCHIVE="${OUTPUT_ROOT}/PurrView-${VERSION_VALUE}-source.tar.xz"

[[ -f "${SOURCE_ARCHIVE}" ]] || {
    printf 'Missing source archive: %s\n' "${SOURCE_ARCHIVE}" >&2
    exit 1
}

rm -rf -- "${WORK_ROOT}"
install -d -o builder -g builder "${WORK_ROOT}"
install -o builder -g builder -m 0644 "${SOURCE_ARCHIVE}" "${WORK_ROOT}/"
SOURCE_SHA256="$(sha256sum "${SOURCE_ARCHIVE}" | awk '{print $1}')"
sed -e "s/@VERSION@/${VERSION_VALUE}/g" \
    -e "s/@SOURCE_SHA256@/${SOURCE_SHA256}/g" \
    "${SOURCE_ROOT}/packaging/arch/PKGBUILD.in" >"${WORK_ROOT}/PKGBUILD"
chown builder:builder "${WORK_ROOT}/PKGBUILD"

runuser -u builder -- bash -lc "cd '${WORK_ROOT}' && makepkg --cleanbuild --noconfirm"
PACKAGE_PATH="$(find "${WORK_ROOT}" -maxdepth 1 -type f \
    -name 'purrview-*.pkg.tar.zst' -print -quit)"
[[ -n "${PACKAGE_PATH}" ]]
install -m 0644 "${PACKAGE_PATH}" "${OUTPUT_ROOT}/"
PACKAGE_PATH="${OUTPUT_ROOT}/$(basename "${PACKAGE_PATH}")"
chown --reference="${OUTPUT_ROOT}" "${PACKAGE_PATH}"

pacman -U --noconfirm "${PACKAGE_PATH}"
[[ "$(purrview --version)" == "PurrView ${VERSION_VALUE}" ]]
[[ "$(impage --version)" == "PurrView ${VERSION_VALUE}" ]]
pacman -Q purrview | grep -Fx "purrview ${VERSION_VALUE}-1" >/dev/null
test -x /usr/bin/purrview
test -x /usr/bin/impage
desktop-file-validate /usr/share/applications/io.github.guedessoftware.PurrView.desktop
appstreamcli validate --no-net /usr/share/metainfo/io.github.guedessoftware.PurrView.metainfo.xml

printf 'Arch package validated: %s\n' "${PACKAGE_PATH}"
