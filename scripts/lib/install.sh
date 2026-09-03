#!/usr/bin/env bash

impage_init_install_paths() {
    if [[ "${IMPAGE_INSTALL_MODE}" == "system" ]]; then
        ((EUID == 0)) || impage_die "--system requires explicit root privileges (run with sudo)."
        IMPAGE_INSTALL_ROOT="/opt/impage"
        IMPAGE_INTEGRATION_PREFIX="/usr/local"
    else
        IMPAGE_INSTALL_ROOT="${HOME}/.local/opt/impage"
        IMPAGE_INTEGRATION_PREFIX="${HOME}/.local"
    fi
    IMPAGE_INSTALL_MANIFEST="${IMPAGE_INSTALL_ROOT}/install-manifest.txt"
    export IMPAGE_INSTALL_ROOT IMPAGE_INTEGRATION_PREFIX IMPAGE_INSTALL_MANIFEST
}

impage_integration_pairs() {
    cat <<EOF
${IMPAGE_INTEGRATION_PREFIX}/bin/impage|${IMPAGE_INSTALL_ROOT}/bin/impage
${IMPAGE_INTEGRATION_PREFIX}/bin/purrview|${IMPAGE_INSTALL_ROOT}/bin/purrview
${IMPAGE_INTEGRATION_PREFIX}/share/applications/io.github.impage.Impage.desktop|${IMPAGE_INSTALL_ROOT}/share/applications/io.github.impage.Impage.desktop
${IMPAGE_INTEGRATION_PREFIX}/share/applications/io.github.impage.Impage.Viewer.desktop|${IMPAGE_INSTALL_ROOT}/share/applications/io.github.impage.Impage.Viewer.desktop
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/scalable/apps/impage.svg|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/scalable/apps/impage.svg
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/scalable/apps/purrview.svg|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/scalable/apps/purrview.svg
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/16x16/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/16x16/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/22x22/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/22x22/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/24x24/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/24x24/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/32x32/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/32x32/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/48x48/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/48x48/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/64x64/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/64x64/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/128x128/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/128x128/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/256x256/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/256x256/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/512x512/apps/purrview.png|${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/512x512/apps/purrview.png
${IMPAGE_INTEGRATION_PREFIX}/share/kio/servicemenus/impage-servicemenu.desktop|${IMPAGE_INSTALL_ROOT}/share/kio/servicemenus/impage-servicemenu.desktop
${IMPAGE_INTEGRATION_PREFIX}/share/metainfo/io.github.impage.Impage.metainfo.xml|${IMPAGE_INSTALL_ROOT}/share/metainfo/io.github.impage.Impage.metainfo.xml
EOF
}

impage_link_one() {
    local destination="$1"
    local source="$2"
    local relative_path="${destination#"${IMPAGE_INTEGRATION_PREFIX}/"}"
    local backup_path="${IMPAGE_INSTALL_ROOT}/integration-backup/${relative_path}"
    local current_target=""
    [[ ! -L "${destination}" ]] || current_target="$(readlink "${destination}")"

    if [[ -e "${destination}" || -L "${destination}" ]] && \
        [[ "${current_target}" != "${source}" ]] && [[ ! -e "${backup_path}" && ! -L "${backup_path}" ]]; then
        mkdir -p "$(dirname "${backup_path}")"
        cp -a -- "${destination}" "${backup_path}"
    fi
    rm -f -- "${destination}"
    ln -s "${source}" "${destination}"
}

impage_link_integration() {
    mkdir -p "${IMPAGE_INTEGRATION_PREFIX}/bin" \
        "${IMPAGE_INTEGRATION_PREFIX}/share/applications" \
        "${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor/scalable/apps" \
        "${IMPAGE_INTEGRATION_PREFIX}/share/kio/servicemenus" \
        "${IMPAGE_INTEGRATION_PREFIX}/share/metainfo"

    local destination
    local source
    while IFS='|' read -r destination source; do
        impage_link_one "${destination}" "${source}"
    done < <(impage_integration_pairs)

    : >"${IMPAGE_INSTALL_MANIFEST}"
    while IFS='|' read -r destination source; do
        local relative_path="${destination#"${IMPAGE_INTEGRATION_PREFIX}/"}"
        local backup_path="integration-backup/${relative_path}"
        [[ -e "${IMPAGE_INSTALL_ROOT}/${backup_path}" || -L "${IMPAGE_INSTALL_ROOT}/${backup_path}" ]] || backup_path="-"
        printf '%s\t%s\t%s\n' "${destination}" "${source}" "${backup_path}" >>"${IMPAGE_INSTALL_MANIFEST}"
    done < <(impage_integration_pairs)
}

impage_activate_stage() {
    local backup_root="${IMPAGE_INSTALL_ROOT}.backup.$$"
    if [[ -e "${IMPAGE_INSTALL_ROOT}" ]]; then
        mv "${IMPAGE_INSTALL_ROOT}" "${backup_root}"
    fi
    if ! mv "${IMPAGE_STAGE_ROOT}" "${IMPAGE_INSTALL_ROOT}"; then
        [[ ! -e "${backup_root}" ]] || mv "${backup_root}" "${IMPAGE_INSTALL_ROOT}"
        impage_die "Could not activate the staged installation."
    fi
    if [[ -d "${backup_root}/integration-backup" ]]; then
        cp -a -- "${backup_root}/integration-backup" "${IMPAGE_INSTALL_ROOT}/"
    fi
    if ! impage_link_integration || ! "${IMPAGE_INSTALL_ROOT}/bin/impage" --version; then
        rm -rf -- "${IMPAGE_INSTALL_ROOT}"
        [[ ! -e "${backup_root}" ]] || mv "${backup_root}" "${IMPAGE_INSTALL_ROOT}"
        impage_die "Installed files failed validation; the previous version was restored."
    fi
    [[ ! -e "${backup_root}" ]] || rm -rf -- "${backup_root}"
    impage_refresh_desktop
    impage_ok "Installed at ${IMPAGE_INSTALL_ROOT}"
    impage_info "Launch with: ${IMPAGE_INTEGRATION_PREFIX}/bin/purrview"
}

impage_uninstall() {
    if [[ ! -f "${IMPAGE_INSTALL_MANIFEST}" ]]; then
        impage_die "No PurrView bootstrap manifest exists at ${IMPAGE_INSTALL_MANIFEST}."
    fi
    local installed_path
    local expected_target
    local backup_relative
    while IFS=$'\t' read -r installed_path expected_target backup_relative; do
        case "${installed_path}" in
        "${IMPAGE_INTEGRATION_PREFIX}"/*)
            if [[ -L "${installed_path}" && "$(readlink "${installed_path}")" == "${expected_target}" ]]; then
                rm -f -- "${installed_path}"
            elif [[ -e "${installed_path}" || -L "${installed_path}" ]]; then
                impage_warn "Preserving externally changed path: ${installed_path}"
                continue
            fi
            if [[ "${backup_relative}" != "-" ]]; then
                local backup_path="${IMPAGE_INSTALL_ROOT}/${backup_relative}"
                if [[ -e "${backup_path}" || -L "${backup_path}" ]]; then
                    mkdir -p "$(dirname "${installed_path}")"
                    mv "${backup_path}" "${installed_path}"
                fi
            fi
            ;;
        *) impage_die "Unsafe path in install manifest: ${installed_path}" ;;
        esac
    done <"${IMPAGE_INSTALL_MANIFEST}"

    case "${IMPAGE_INSTALL_ROOT}" in
    "${HOME}/.local/opt/impage" | /opt/impage) rm -rf -- "${IMPAGE_INSTALL_ROOT}" ;;
    *) impage_die "Refusing to remove unexpected root ${IMPAGE_INSTALL_ROOT}." ;;
    esac
    impage_refresh_desktop
    impage_ok "PurrView bootstrap installation removed; user settings were preserved."
}
