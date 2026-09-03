#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE_ROOT=/src
BUILD_ROOT=/tmp/purrview-build-rpm
PACKAGE_ROOT=/tmp/purrview-package-rpm
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
cpack --config "${BUILD_ROOT}/CPackConfig.cmake" -G RPM -B "${PACKAGE_ROOT}"

BUILT_PACKAGE="$(find "${PACKAGE_ROOT}" -maxdepth 1 -type f \
    -name "purrview-${VERSION_VALUE}-*.rpm" -print -quit)"
[[ -n "${BUILT_PACKAGE}" ]]
install -m 0644 "${BUILT_PACKAGE}" "${OUTPUT_ROOT}/"
PACKAGE_PATH="${OUTPUT_ROOT}/$(basename "${BUILT_PACKAGE}")"
chown --reference="${OUTPUT_ROOT}" "${PACKAGE_PATH}"
rpm -qip "${PACKAGE_PATH}" >/dev/null
rpm -qlp "${PACKAGE_PATH}" | grep -F '/usr/bin/purrview' >/dev/null

dnf -y -q install "${PACKAGE_PATH}"
[[ "$(purrview --version)" == "PurrView ${VERSION_VALUE}" ]]
desktop-file-validate /usr/share/applications/io.github.impage.Impage.desktop
appstreamcli validate --no-net /usr/share/metainfo/io.github.impage.Impage.metainfo.xml

printf 'RPM package validated on AlmaLinux 9: %s\n' "${PACKAGE_PATH}"
