#!/usr/bin/env bash

impage_detect_distribution() {
    local os_release="${IMPAGE_OS_RELEASE_FILE:-/etc/os-release}"
    [[ -r "${os_release}" ]] || impage_die "Cannot read ${os_release}."

    local ID=""
    local ID_LIKE=""
    local VERSION_ID=""
    local PRETTY_NAME=""
    # /etc/os-release is the standard machine-readable distribution contract.
    # shellcheck disable=SC1090
    source "${os_release}"

    IMPAGE_DISTRO_ID="${ID,,}"
    IMPAGE_DISTRO_LIKE="${ID_LIKE,,}"
    IMPAGE_DISTRO_VERSION="${VERSION_ID:-unknown}"
    IMPAGE_DISTRO_NAME="${PRETTY_NAME:-${ID} ${VERSION_ID:-}}"

    case " ${IMPAGE_DISTRO_ID} ${IMPAGE_DISTRO_LIKE} " in
    *" ubuntu "*) IMPAGE_DISTRO_FAMILY="ubuntu" ;;
    *" debian "*) IMPAGE_DISTRO_FAMILY="debian" ;;
    *" fedora "* | *" rhel "*) IMPAGE_DISTRO_FAMILY="fedora" ;;
    *" arch "*) IMPAGE_DISTRO_FAMILY="arch" ;;
    *) impage_die "Unsupported distribution: ${IMPAGE_DISTRO_NAME}." ;;
    esac
    export IMPAGE_DISTRO_ID IMPAGE_DISTRO_LIKE IMPAGE_DISTRO_VERSION
    export IMPAGE_DISTRO_NAME IMPAGE_DISTRO_FAMILY
}

impage_detect_architecture() {
    IMPAGE_ARCHITECTURE="${IMPAGE_ARCH_OVERRIDE:-$(uname -m)}"
    case "${IMPAGE_ARCHITECTURE}" in
    x86_64) ;;
    aarch64 | arm64)
        impage_die "aarch64 is recognized but has not been validated yet."
        ;;
    *) impage_die "Unsupported architecture: ${IMPAGE_ARCHITECTURE}." ;;
    esac
    export IMPAGE_ARCHITECTURE
}
