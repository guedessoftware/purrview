#!/usr/bin/env bash

purrview_detect_distribution() {
    local os_release="${PURRVIEW_OS_RELEASE_FILE:-/etc/os-release}"
    [[ -r "${os_release}" ]] || purrview_die "Cannot read ${os_release}."

    local ID=""
    local ID_LIKE=""
    local VERSION_ID=""
    local PRETTY_NAME=""
    # /etc/os-release is the standard machine-readable distribution contract.
    # shellcheck disable=SC1090
    source "${os_release}"

    PURRVIEW_DISTRO_ID="${ID,,}"
    PURRVIEW_DISTRO_LIKE="${ID_LIKE,,}"
    PURRVIEW_DISTRO_VERSION="${VERSION_ID:-unknown}"
    PURRVIEW_DISTRO_NAME="${PRETTY_NAME:-${ID} ${VERSION_ID:-}}"

    case " ${PURRVIEW_DISTRO_ID} ${PURRVIEW_DISTRO_LIKE} " in
    *" ubuntu "*) PURRVIEW_DISTRO_FAMILY="ubuntu" ;;
    *" debian "*) PURRVIEW_DISTRO_FAMILY="debian" ;;
    *" fedora "* | *" rhel "*) PURRVIEW_DISTRO_FAMILY="fedora" ;;
    *" arch "*) PURRVIEW_DISTRO_FAMILY="arch" ;;
    *) purrview_die "Unsupported distribution: ${PURRVIEW_DISTRO_NAME}." ;;
    esac
    export PURRVIEW_DISTRO_ID PURRVIEW_DISTRO_LIKE PURRVIEW_DISTRO_VERSION
    export PURRVIEW_DISTRO_NAME PURRVIEW_DISTRO_FAMILY
}

purrview_detect_architecture() {
    PURRVIEW_ARCHITECTURE="${PURRVIEW_ARCH_OVERRIDE:-$(uname -m)}"
    case "${PURRVIEW_ARCHITECTURE}" in
    x86_64) ;;
    aarch64 | arm64)
        purrview_die "aarch64 is recognized but has not been validated yet."
        ;;
    *) purrview_die "Unsupported architecture: ${PURRVIEW_ARCHITECTURE}." ;;
    esac
    export PURRVIEW_ARCHITECTURE
}
