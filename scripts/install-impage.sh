#!/usr/bin/env bash
set -Eeuo pipefail

IMPAGE_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
IMPAGE_SOURCE_ROOT="$(cd -- "${IMPAGE_SCRIPT_DIR}/.." && pwd)"
export IMPAGE_SCRIPT_DIR IMPAGE_SOURCE_ROOT

# shellcheck source=lib/common.sh
source "${IMPAGE_SCRIPT_DIR}/lib/common.sh"
# shellcheck source=lib/distro.sh
source "${IMPAGE_SCRIPT_DIR}/lib/distro.sh"
# shellcheck source=lib/dependencies.sh
source "${IMPAGE_SCRIPT_DIR}/lib/dependencies.sh"
# shellcheck source=lib/build.sh
source "${IMPAGE_SCRIPT_DIR}/lib/build.sh"
# shellcheck source=lib/desktop.sh
source "${IMPAGE_SCRIPT_DIR}/lib/desktop.sh"
# shellcheck source=lib/install.sh
source "${IMPAGE_SCRIPT_DIR}/lib/install.sh"

IMPAGE_INSTALL_MODE="user"
IMPAGE_NON_INTERACTIVE=0
IMPAGE_CHECK_ONLY=0
IMPAGE_UNINSTALL=0
IMPAGE_UPGRADE=0
IMPAGE_VERIFY_BUILD=0
IMPAGE_SET_DEFAULT_VIEWER=0

usage() {
    cat <<'EOF'
Usage: scripts/install-impage.sh [options]

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
    --user) IMPAGE_INSTALL_MODE="user" ;;
    --system) IMPAGE_INSTALL_MODE="system" ;;
    --check) IMPAGE_CHECK_ONLY=1 ;;
    --non-interactive) IMPAGE_NON_INTERACTIVE=1 ;;
    --upgrade) IMPAGE_UPGRADE=1 ;;
    --verify) IMPAGE_VERIFY_BUILD=1 ;;
    --set-default-viewer) IMPAGE_SET_DEFAULT_VIEWER=1 ;;
    --uninstall) IMPAGE_UNINSTALL=1 ;;
    --help | -h) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
    esac
    shift
done
export IMPAGE_INSTALL_MODE IMPAGE_NON_INTERACTIVE IMPAGE_CHECK_ONLY
export IMPAGE_UNINSTALL IMPAGE_UPGRADE IMPAGE_VERIFY_BUILD IMPAGE_SET_DEFAULT_VIEWER

impage_setup_log
impage_info "PurrView Installer"
impage_detect_distribution
impage_detect_architecture
impage_init_install_paths
if ((IMPAGE_SET_DEFAULT_VIEWER)) && [[ "${IMPAGE_INSTALL_MODE}" != "user" ]]; then
    impage_die "--set-default-viewer is a per-user operation and cannot be combined with --system."
fi
impage_info "Distribution: ${IMPAGE_DISTRO_NAME} (${IMPAGE_DISTRO_FAMILY})"
impage_info "Architecture: ${IMPAGE_ARCHITECTURE}"
impage_info "Mode: ${IMPAGE_INSTALL_MODE}"

if ((EUID == 0)) && [[ "${IMPAGE_INSTALL_MODE}" == "user" && "${HOME}" == "/root" ]]; then
    if ((IMPAGE_CHECK_ONLY)); then
        impage_warn "User installation must be run as the regular desktop user, not root."
    elif ((!IMPAGE_UNINSTALL)); then
        impage_die "Refusing a user installation in /root. Exit the root shell and run this script as your regular desktop user; it will request sudo only when dependencies are missing. Use --system only for an intentional system-wide installation."
    fi
fi

if ((IMPAGE_UNINSTALL)); then
    impage_uninstall
    exit 0
fi

if ((IMPAGE_UPGRADE)) && [[ ! -f "${IMPAGE_INSTALL_MANIFEST}" ]]; then
    impage_die "--upgrade requires an existing bootstrap installation."
fi

impage_info "Checking dependencies..."
impage_check_dependencies
if ((!IMPAGE_DEPENDENCIES_OK)); then
    if ((IMPAGE_CHECK_ONLY)); then
        impage_info "Planned packages: ${IMPAGE_MISSING_PACKAGES[*]}"
        exit 1
    fi
    if ((IMPAGE_NON_INTERACTIVE)); then
        impage_die "Missing dependencies; non-interactive mode never runs a package manager."
    fi
    impage_install_dependencies
    impage_check_dependencies
    ((IMPAGE_DEPENDENCIES_OK)) || impage_die "Dependencies remain incomplete."
fi

if ((IMPAGE_CHECK_ONLY)); then
    impage_info "Install root: ${IMPAGE_INSTALL_ROOT}"
    impage_info "Desktop integration: ${IMPAGE_INTEGRATION_PREFIX}"
    if ((IMPAGE_SET_DEFAULT_VIEWER)); then
        impage_info "Default Viewer: PurrView for supported image formats"
    fi
    impage_ok "No changes were made"
    exit 0
fi

impage_build_stage
impage_activate_stage
if ((IMPAGE_SET_DEFAULT_VIEWER)); then
    impage_set_default_viewer || \
        impage_warn "PurrView was installed, but one or more default associations could not be changed."
fi
impage_info "Completed. Log: ${IMPAGE_INSTALL_LOG}"
