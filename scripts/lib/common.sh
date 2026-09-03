#!/usr/bin/env bash

impage_info() {
    printf '%s\n' "$*"
}

impage_ok() {
    printf '✓ %s\n' "$*"
}

impage_warn() {
    printf '! %s\n' "$*" >&2
}

impage_die() {
    printf 'Installation failed: %s\nLog: %s\n' "$*" "${IMPAGE_INSTALL_LOG:-not created}" >&2
    exit 1
}

impage_setup_log() {
    local cache_root="${XDG_CACHE_HOME:-${HOME}/.cache}/impage"
    mkdir -p "${cache_root}"
    IMPAGE_INSTALL_LOG="${cache_root}/install.log"
    export IMPAGE_INSTALL_LOG
    : >"${IMPAGE_INSTALL_LOG}"
    exec > >(tee -a "${IMPAGE_INSTALL_LOG}") 2>&1
}

impage_version_ge() {
    local candidate="$1"
    local minimum="$2"
    [[ "$(printf '%s\n%s\n' "${minimum}" "${candidate}" | sort -V | head -n 1)" == "${minimum}" ]]
}

impage_lock_value() {
    local key="$1"
    sed -n "s/^set(${key} \"\\([^\"]*\\)\")$/\\1/p" \
        "${IMPAGE_SOURCE_ROOT}/cmake/DependencyVersions.cmake"
}

impage_confirm() {
    local prompt="$1"
    if [[ "${IMPAGE_NON_INTERACTIVE}" == "1" ]]; then
        return 1
    fi
    local answer
    read -r -p "${prompt} [S/n] " answer
    [[ -z "${answer}" || "${answer}" =~ ^[SsYy]$ ]]
}
