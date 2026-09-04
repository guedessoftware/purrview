#!/usr/bin/env bash
set -Eeuo pipefail

PURRVIEW_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PURRVIEW_SOURCE_ROOT="$(cd -- "${PURRVIEW_SCRIPT_DIR}/.." && pwd)"
export PURRVIEW_SCRIPT_DIR PURRVIEW_SOURCE_ROOT

# shellcheck source=lib/common.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/common.sh"
# shellcheck source=lib/distro.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/distro.sh"
# shellcheck source=lib/dependencies.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/dependencies.sh"
# shellcheck source=lib/build.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/build.sh"
# shellcheck source=lib/desktop.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/desktop.sh"
# shellcheck source=lib/install.sh
source "${PURRVIEW_SCRIPT_DIR}/lib/install.sh"

PURRVIEW_INSTALL_MODE="user"
PURRVIEW_NON_INTERACTIVE=0
PURRVIEW_CHECK_ONLY=0
PURRVIEW_UNINSTALL=0
PURRVIEW_UPGRADE=0
PURRVIEW_VERIFY_BUILD=0
PURRVIEW_SET_DEFAULT_VIEWER=0

usage() {
    cat <<'EOF'
Usage: scripts/install-purrview.sh [options]

  --user             Install for the current user (default)
  --system           Install under /opt and /usr/local; requires sudo/root
  --check            Report platform, dependencies and planned paths only
  --non-interactive  Never invoke a package manager or prompt
  --upgrade          Replace an existing bootstrap installation atomically
  --verify           Also build and run the local test suite (slower)
  --set-default-viewer
                     Make PurrView the default only for its supported image formats
  --uninstall        Remove only files recorded by the bootstrap manifest
  --help             Show this help
EOF
}

while (($# > 0)); do
    case "$1" in
    --user) PURRVIEW_INSTALL_MODE="user" ;;
    --system) PURRVIEW_INSTALL_MODE="system" ;;
    --check) PURRVIEW_CHECK_ONLY=1 ;;
    --non-interactive) PURRVIEW_NON_INTERACTIVE=1 ;;
    --upgrade) PURRVIEW_UPGRADE=1 ;;
    --verify) PURRVIEW_VERIFY_BUILD=1 ;;
    --set-default-viewer) PURRVIEW_SET_DEFAULT_VIEWER=1 ;;
    --uninstall) PURRVIEW_UNINSTALL=1 ;;
    --help | -h) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
    esac
    shift
done
export PURRVIEW_INSTALL_MODE PURRVIEW_NON_INTERACTIVE PURRVIEW_CHECK_ONLY
export PURRVIEW_UNINSTALL PURRVIEW_UPGRADE PURRVIEW_VERIFY_BUILD PURRVIEW_SET_DEFAULT_VIEWER

purrview_setup_log
purrview_info "PurrView Installer"
purrview_detect_distribution
purrview_detect_architecture
purrview_init_install_paths
if ((PURRVIEW_SET_DEFAULT_VIEWER)) && [[ "${PURRVIEW_INSTALL_MODE}" != "user" ]]; then
    purrview_die "--set-default-viewer is a per-user operation and cannot be combined with --system."
fi
purrview_info "Distribution: ${PURRVIEW_DISTRO_NAME} (${PURRVIEW_DISTRO_FAMILY})"
purrview_info "Architecture: ${PURRVIEW_ARCHITECTURE}"
purrview_info "Mode: ${PURRVIEW_INSTALL_MODE}"

if ((EUID == 0)) && [[ "${PURRVIEW_INSTALL_MODE}" == "user" && "${HOME}" == "/root" ]]; then
    if ((PURRVIEW_CHECK_ONLY)); then
        purrview_warn "User installation must be run as the regular desktop user, not root."
    elif ((!PURRVIEW_UNINSTALL)); then
        purrview_die "Refusing a user installation in /root. Exit the root shell and run this script as your regular desktop user; it will request sudo only when dependencies are missing. Use --system only for an intentional system-wide installation."
    fi
fi

if ((PURRVIEW_UNINSTALL)); then
    purrview_uninstall
    exit 0
fi

if ((PURRVIEW_UPGRADE)) && ! purrview_has_existing_install; then
    purrview_die "--upgrade requires an existing bootstrap installation."
fi

purrview_info "Checking dependencies..."
purrview_check_dependencies
if ((!PURRVIEW_DEPENDENCIES_OK)); then
    if ((PURRVIEW_CHECK_ONLY)); then
        purrview_info "Planned packages: ${PURRVIEW_MISSING_PACKAGES[*]}"
        exit 1
    fi
    if ((PURRVIEW_NON_INTERACTIVE)); then
        purrview_die "Missing dependencies; non-interactive mode never runs a package manager."
    fi
    purrview_install_dependencies
    purrview_check_dependencies
    ((PURRVIEW_DEPENDENCIES_OK)) || purrview_die "Dependencies remain incomplete."
fi

if ((PURRVIEW_CHECK_ONLY)); then
    purrview_info "Install root: ${PURRVIEW_INSTALL_ROOT}"
    purrview_info "Desktop integration: ${PURRVIEW_INTEGRATION_PREFIX}"
    if ((PURRVIEW_SET_DEFAULT_VIEWER)); then
        purrview_info "Default Viewer: PurrView for supported image formats"
    fi
    purrview_ok "No changes were made"
    exit 0
fi

purrview_build_stage
purrview_activate_stage
if ((PURRVIEW_SET_DEFAULT_VIEWER)); then
    purrview_set_default_viewer || \
        purrview_warn "PurrView was installed, but one or more default associations could not be changed."
fi
purrview_info "Completed. Log: ${PURRVIEW_INSTALL_LOG}"
