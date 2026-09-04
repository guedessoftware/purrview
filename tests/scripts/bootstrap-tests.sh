#!/usr/bin/env bash
set -Eeuo pipefail

TEST_ROOT="$(mktemp -d)"
trap 'rm -rf -- "${TEST_ROOT}"' EXIT
PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
export PURRVIEW_SOURCE_ROOT="${PROJECT_ROOT}"
export PURRVIEW_INSTALL_LOG="${TEST_ROOT}/install.log"

# shellcheck source=../../scripts/lib/common.sh
source "${PROJECT_ROOT}/scripts/lib/common.sh"
# shellcheck source=../../scripts/lib/distro.sh
source "${PROJECT_ROOT}/scripts/lib/distro.sh"
# shellcheck source=../../scripts/lib/desktop.sh
source "${PROJECT_ROOT}/scripts/lib/desktop.sh"
# shellcheck source=../../scripts/lib/build.sh
source "${PROJECT_ROOT}/scripts/lib/build.sh"
# shellcheck source=../../scripts/lib/install.sh
source "${PROJECT_ROOT}/scripts/lib/install.sh"

check_family() {
    local expected="$1"
    local contents="$2"
    local fixture="${TEST_ROOT}/${expected}.os-release"
    printf '%s\n' "${contents}" >"${fixture}"
    PURRVIEW_OS_RELEASE_FILE="${fixture}" purrview_detect_distribution
    [[ "${PURRVIEW_DISTRO_FAMILY}" == "${expected}" ]]
}

check_family ubuntu $'ID=ubuntu\nVERSION_ID="26.04"\nPRETTY_NAME="Ubuntu 26.04 LTS"'
check_family debian $'ID=debian\nVERSION_ID="13"\nPRETTY_NAME="Debian GNU/Linux 13"'
check_family fedora $'ID=fedora\nVERSION_ID="44"\nPRETTY_NAME="Fedora Linux 44"'
check_family arch $'ID=garuda\nID_LIKE=arch\nPRETTY_NAME="Garuda Linux"'

PURRVIEW_ARCH_OVERRIDE=x86_64 purrview_detect_architecture
[[ "${PURRVIEW_ARCHITECTURE}" == "x86_64" ]]

ORIGINAL_HOME="${HOME}"
HOME="${TEST_ROOT}/home"
mkdir -p "${HOME}"
PURRVIEW_INSTALL_MODE=user
purrview_init_install_paths
[[ "${PURRVIEW_INSTALL_ROOT}" == "${HOME}/.local/opt/purrview" ]]
[[ "${PURRVIEW_LEGACY_INSTALL_ROOT}" == "${HOME}/.local/opt/impage" ]]
[[ ! -e "${PURRVIEW_INSTALL_MANIFEST}" ]]
mkdir -p "${PURRVIEW_LEGACY_INSTALL_ROOT}"
: >"${PURRVIEW_LEGACY_INSTALL_MANIFEST}"
purrview_has_existing_install
HOME="${ORIGINAL_HOME}"

PURRVIEW_VERIFY_BUILD=0
[[ "$(purrview_build_testing_value)" == "OFF" ]]
PURRVIEW_VERIFY_BUILD=1
[[ "$(purrview_build_testing_value)" == "ON" ]]

mapfile -t SUPPORTED_MIME_TYPES < <(purrview_supported_mime_types)
[[ "${#SUPPORTED_MIME_TYPES[@]}" == 10 ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/avif "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/heif "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/heic "* ]]
[[ " ${SUPPORTED_MIME_TYPES[*]} " == *" image/x-icns "* ]]

PURRVIEW_STAGE_ROOT="${TEST_ROOT}/stage"
PURRVIEW_INSTALL_ROOT="/home/tester/.local/opt/purrview"
mkdir -p "${PURRVIEW_STAGE_ROOT}/share/applications" "${PURRVIEW_STAGE_ROOT}/share/kio/servicemenus"
cp "${PROJECT_ROOT}/resources/linux/io.github.guedessoftware.PurrView.desktop" \
    "${PROJECT_ROOT}/resources/linux/io.github.guedessoftware.PurrView.Viewer.desktop" \
    "${PURRVIEW_STAGE_ROOT}/share/applications/"
cp "${PROJECT_ROOT}/resources/linux/purrview-servicemenu.desktop" \
    "${PURRVIEW_STAGE_ROOT}/share/kio/servicemenus/"
purrview_configure_desktop_launchers

grep -Fqx 'Name=PurrView Print' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
grep -Fqx 'Name[pt_BR]=PurrView Impressão' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
grep -Fqx 'Name=PurrView' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.Viewer.desktop"
grep -Fqx 'Exec=/home/tester/.local/opt/purrview/bin/purrview %F' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
grep -Fqx 'TryExec=/home/tester/.local/opt/purrview/bin/purrview' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
grep -Fqx 'Icon=/home/tester/.local/opt/purrview/share/icons/hicolor/256x256/apps/purrview.png' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
grep -Fqx 'Exec=/home/tester/.local/opt/purrview/bin/purrview --viewer %F' \
    "${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.Viewer.desktop"
grep -Fqx 'Exec=/home/tester/.local/opt/purrview/bin/purrview --compose %F' \
    "${PURRVIEW_STAGE_ROOT}/share/kio/servicemenus/purrview-servicemenu.desktop"
grep -Fqx 'Icon=/home/tester/.local/opt/purrview/share/icons/hicolor/256x256/apps/purrview.png' \
    "${PURRVIEW_STAGE_ROOT}/share/kio/servicemenus/purrview-servicemenu.desktop"
grep -Fq '/purrview" "$@"' "${PROJECT_ROOT}/resources/bin/impage"
grep -Fq '/install-purrview.sh" "$@"' "${PROJECT_ROOT}/scripts/install-impage.sh"

# Simulate an upgrade from the historical bootstrap root. Shared command links
# must point to PurrView afterwards, while old desktop-only links disappear.
HOME="${TEST_ROOT}/migration-home"
mkdir -p "${HOME}"
PURRVIEW_INSTALL_MODE=user
purrview_init_install_paths
mkdir -p "${PURRVIEW_LEGACY_INSTALL_ROOT}/bin" \
    "${PURRVIEW_LEGACY_INSTALL_ROOT}/share/applications" \
    "${PURRVIEW_INSTALL_ROOT}/bin" \
    "${PURRVIEW_INTEGRATION_PREFIX}/bin" \
    "${PURRVIEW_INTEGRATION_PREFIX}/share/applications"
: >"${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/purrview"
: >"${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/impage"
: >"${PURRVIEW_LEGACY_INSTALL_ROOT}/share/applications/io.github.impage.Impage.desktop"
ln -s "${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/purrview" \
    "${PURRVIEW_INTEGRATION_PREFIX}/bin/purrview"
ln -s "${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/impage" \
    "${PURRVIEW_INTEGRATION_PREFIX}/bin/impage"
ln -s "${PURRVIEW_LEGACY_INSTALL_ROOT}/share/applications/io.github.impage.Impage.desktop" \
    "${PURRVIEW_INTEGRATION_PREFIX}/share/applications/io.github.impage.Impage.desktop"
cat >"${PURRVIEW_LEGACY_INSTALL_MANIFEST}" <<EOF
${PURRVIEW_INTEGRATION_PREFIX}/bin/purrview	${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/purrview	-
${PURRVIEW_INTEGRATION_PREFIX}/bin/impage	${PURRVIEW_LEGACY_INSTALL_ROOT}/bin/impage	-
${PURRVIEW_INTEGRATION_PREFIX}/share/applications/io.github.impage.Impage.desktop	${PURRVIEW_LEGACY_INSTALL_ROOT}/share/applications/io.github.impage.Impage.desktop	-
EOF
while IFS='|' read -r destination source; do
    mkdir -p "$(dirname "${destination}")" "$(dirname "${source}")"
    : >"${source}"
done < <(purrview_integration_pairs)
purrview_link_integration
purrview_cleanup_legacy_install
[[ "$(readlink "${PURRVIEW_INTEGRATION_PREFIX}/bin/purrview")" == \
   "${PURRVIEW_INSTALL_ROOT}/bin/purrview" ]]
[[ "$(readlink "${PURRVIEW_INTEGRATION_PREFIX}/bin/impage")" == \
   "${PURRVIEW_INSTALL_ROOT}/bin/impage" ]]
[[ ! -e "${PURRVIEW_INTEGRATION_PREFIX}/share/applications/io.github.impage.Impage.desktop" ]]
[[ ! -e "${PURRVIEW_LEGACY_INSTALL_ROOT}" ]]
HOME="${ORIGINAL_HOME}"
printf 'Bootstrap distribution tests passed.\n'
