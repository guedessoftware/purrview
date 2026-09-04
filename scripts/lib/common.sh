#!/usr/bin/env bash

purrview_info() {
    printf '%s\n' "$*"
}

purrview_ok() {
    printf '✓ %s\n' "$*"
}

purrview_warn() {
    printf '! %s\n' "$*" >&2
}

purrview_die() {
    printf 'Installation failed: %s\nLog: %s\n' "$*" "${PURRVIEW_INSTALL_LOG:-not created}" >&2
    exit 1
}

purrview_setup_log() {
    local cache_root="${XDG_CACHE_HOME:-${HOME}/.cache}/purrview"
    mkdir -p "${cache_root}"
    PURRVIEW_INSTALL_LOG="${cache_root}/install.log"
    export PURRVIEW_INSTALL_LOG
    : >"${PURRVIEW_INSTALL_LOG}"
    exec > >(tee -a "${PURRVIEW_INSTALL_LOG}") 2>&1
}

purrview_version_ge() {
    local candidate="$1"
    local minimum="$2"
    [[ "$(printf '%s\n%s\n' "${minimum}" "${candidate}" | sort -V | head -n 1)" == "${minimum}" ]]
}

purrview_lock_value() {
    local key="$1"
    sed -n "s/^set(${key} \"\\([^\"]*\\)\")$/\\1/p" \
        "${PURRVIEW_SOURCE_ROOT}/cmake/DependencyVersions.cmake"
}

purrview_confirm() {
    local prompt="$1"
    if [[ "${PURRVIEW_NON_INTERACTIVE}" == "1" ]]; then
        return 1
    fi
    local answer
    read -r -p "${prompt} [S/n] " answer
    [[ -z "${answer}" || "${answer}" =~ ^[SsYy]$ ]]
}
