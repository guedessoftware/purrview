#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE_ROOT=/src
BUILD_ROOT=/tmp/purrview-build-deb
PACKAGE_ROOT=/tmp/purrview-package-deb
OUTPUT_ROOT=/out
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"

rm -rf -- "${BUILD_ROOT}" "${PACKAGE_ROOT}"
cmake -S "${SOURCE_ROOT}" -B "${BUILD_ROOT}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DBUILD_TESTING=ON \
    -DIMPAGE_WITH_EXIV2=OFF
cmake --build "${BUILD_ROOT}" --parallel "$(nproc)"
ctest --test-dir "${BUILD_ROOT}" --output-on-failure
cpack --config "${BUILD_ROOT}/CPackConfig.cmake" -G DEB -B "${PACKAGE_ROOT}"

BUILT_PACKAGE="$(find "${PACKAGE_ROOT}" -maxdepth 1 -type f \
    -name "purrview_${VERSION_VALUE}_*.deb" -print -quit)"
[[ -n "${BUILT_PACKAGE}" ]]
install -m 0644 "${BUILT_PACKAGE}" "${OUTPUT_ROOT}/"
PACKAGE_PATH="${OUTPUT_ROOT}/$(basename "${BUILT_PACKAGE}")"
chown --reference="${OUTPUT_ROOT}" "${PACKAGE_PATH}"
dpkg-deb --info "${PACKAGE_PATH}" >/dev/null
dpkg-deb --contents "${PACKAGE_PATH}" | grep -F './usr/bin/purrview' >/dev/null

apt-get update -qq
apt-get install -y --no-install-recommends "${PACKAGE_PATH}"
[[ "$(purrview --version)" == "PurrView ${VERSION_VALUE}" ]]
desktop-file-validate /usr/share/applications/io.github.impage.Impage.desktop
appstreamcli validate --no-net /usr/share/metainfo/io.github.impage.Impage.metainfo.xml

printf 'DEB package validated on Ubuntu 24.04: %s\n' "${PACKAGE_PATH}"
