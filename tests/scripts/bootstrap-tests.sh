#!/usr/bin/env bash
set -Eeuo pipefail

TEST_ROOT="$(mktemp -d)"
trap 'rm -rf -- "${TEST_ROOT}"' EXIT
PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
export IMPAGE_SOURCE_ROOT="${PROJECT_ROOT}"
export IMPAGE_INSTALL_LOG="${TEST_ROOT}/install.log"

# shellcheck source=../../scripts/lib/common.sh
source "${PROJECT_ROOT}/scripts/lib/common.sh"
# shellcheck source=../../scripts/lib/distro.sh
source "${PROJECT_ROOT}/scripts/lib/distro.sh"
# shellcheck source=../../scripts/lib/desktop.sh
source "${PROJECT_ROOT}/scripts/lib/desktop.sh"
# shellcheck source=../../scripts/lib/build.sh
source "${PROJECT_ROOT}/scripts/lib/build.sh"

check_family() {
    local expected="$1"
    local contents="$2"
    local fixture="${TEST_ROOT}/${expected}.os-release"
    printf '%s\n' "${contents}" >"${fixture}"
    IMPAGE_OS_RELEASE_FILE="${fixture}" impage_detect_distribution
    [[ "${IMPAGE_DISTRO_FAMILY}" == "${expected}" ]]
}

check_family ubuntu $'ID=ubuntu\nVERSION_ID="26.04"\nPRETTY_NAME="Ubuntu 26.04 LTS"'
check_family debian $'ID=debian\nVERSION_ID="13"\nPRETTY_NAME="Debian GNU/Linux 13"'
check_family fedora $'ID=fedora\nVERSION_ID="44"\nPRETTY_NAME="Fedora Linux 44"'
check_family arch $'ID=garuda\nID_LIKE=arch\nPRETTY_NAME="Garuda Linux"'

IMPAGE_ARCH_OVERRIDE=x86_64 impage_detect_architecture
[[ "${IMPAGE_ARCHITECTURE}" == "x86_64" ]]

IMPAGE_VERIFY_BUILD=0
[[ "$(impage_build_testing_value)" == "OFF" ]]
IMPAGE_VERIFY_BUILD=1
[[ "$(impage_build_testing_value)" == "ON" ]]

mapfile -t SUPPORTED_MIME_TYPES < <(impage_supported_mime_types)
[[ "${#SUPPORTED_MIME_TYPES[@]}" == 10 ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/avif "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/heif "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/heic "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/x-icns "* ]]

IMPAGE_STAGE_ROOT="${TEST_ROOT}/stage"
IMPAGE_INSTALL_ROOT="/home/tester/.local/opt/impage"
mkdir -p "${IMPAGE_STAGE_ROOT}/share/applications" "${IMPAGE_STAGE_ROOT}/share/kio/servicemenus"
cp "${PROJECT_ROOT}/resources/linux/io.github.impage.Impage.desktop" \
    "${PROJECT_ROOT}/resources/linux/io.github.impage.Impage.Viewer.desktop" \
    "${IMPAGE_STAGE_ROOT}/share/applications/"
cp "${PROJECT_ROOT}/resources/linux/impage-servicemenu.desktop" \
    "${IMPAGE_STAGE_ROOT}/share/kio/servicemenus/"
impage_configure_desktop_launchers

grep -Fqx 'Exec=/home/tester/.local/opt/impage/bin/purrview %F' \
    "${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.desktop"
grep -Fqx 'TryExec=/home/tester/.local/opt/impage/bin/purrview' \
    "${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.desktop"
grep -Fqx 'Icon=/home/tester/.local/opt/impage/share/icons/hicolor/256x256/apps/purrview.png' \
    "${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.desktop"
grep -Fqx 'Exec=/home/tester/.local/opt/impage/bin/purrview --viewer %F' \
    "${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.Viewer.desktop"
grep -Fqx 'Exec=/home/tester/.local/opt/impage/bin/purrview --compose %F' \
    "${IMPAGE_STAGE_ROOT}/share/kio/servicemenus/impage-servicemenu.desktop"
grep -Fqx 'Icon=/home/tester/.local/opt/impage/share/icons/hicolor/256x256/apps/purrview.png' \
    "${IMPAGE_STAGE_ROOT}/share/kio/servicemenus/impage-servicemenu.desktop"
printf 'Bootstrap distribution tests passed.\n'
