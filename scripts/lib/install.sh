#!/usr/bin/env bash

purrview_init_install_paths() {
    if [[ "${PURRVIEW_INSTALL_MODE}" == "system" ]]; then
        ((EUID == 0)) || purrview_die "--system requires explicit root privileges (run with sudo)."
        PURRVIEW_INSTALL_ROOT="/opt/purrview"
        PURRVIEW_LEGACY_INSTALL_ROOT="/opt/impage"
        PURRVIEW_INTEGRATION_PREFIX="/usr/local"
    else
        PURRVIEW_INSTALL_ROOT="${HOME}/.local/opt/purrview"
        PURRVIEW_LEGACY_INSTALL_ROOT="${HOME}/.local/opt/impage"
        PURRVIEW_INTEGRATION_PREFIX="${HOME}/.local"
    fi
    PURRVIEW_INSTALL_MANIFEST="${PURRVIEW_INSTALL_ROOT}/install-manifest.txt"
    PURRVIEW_LEGACY_INSTALL_MANIFEST="${PURRVIEW_LEGACY_INSTALL_ROOT}/install-manifest.txt"
    export PURRVIEW_INSTALL_ROOT PURRVIEW_LEGACY_INSTALL_ROOT PURRVIEW_INTEGRATION_PREFIX
    export PURRVIEW_INSTALL_MANIFEST PURRVIEW_LEGACY_INSTALL_MANIFEST
}

purrview_has_existing_install() {
    [[ -f "${PURRVIEW_INSTALL_MANIFEST}" || -f "${PURRVIEW_LEGACY_INSTALL_MANIFEST}" ]]
}

purrview_integration_pairs() {
    cat <<EOF
${PURRVIEW_INTEGRATION_PREFIX}/bin/purrview|${PURRVIEW_INSTALL_ROOT}/bin/purrview
${PURRVIEW_INTEGRATION_PREFIX}/bin/impage|${PURRVIEW_INSTALL_ROOT}/bin/impage
${PURRVIEW_INTEGRATION_PREFIX}/share/applications/io.github.guedessoftware.PurrView.desktop|${PURRVIEW_INSTALL_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop
${PURRVIEW_INTEGRATION_PREFIX}/share/applications/io.github.guedessoftware.PurrView.Viewer.desktop|${PURRVIEW_INSTALL_ROOT}/share/applications/io.github.guedessoftware.PurrView.Viewer.desktop
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/scalable/apps/purrview.svg|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/scalable/apps/purrview.svg
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/16x16/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/16x16/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/22x22/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/22x22/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/24x24/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/24x24/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/32x32/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/32x32/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/48x48/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/48x48/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/64x64/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/64x64/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/128x128/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/128x128/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/256x256/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/256x256/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/512x512/apps/purrview.png|${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/512x512/apps/purrview.png
${PURRVIEW_INTEGRATION_PREFIX}/share/kio/servicemenus/purrview-servicemenu.desktop|${PURRVIEW_INSTALL_ROOT}/share/kio/servicemenus/purrview-servicemenu.desktop
${PURRVIEW_INTEGRATION_PREFIX}/share/metainfo/io.github.guedessoftware.PurrView.metainfo.xml|${PURRVIEW_INSTALL_ROOT}/share/metainfo/io.github.guedessoftware.PurrView.metainfo.xml
EOF
}

purrview_link_one() {
    local destination="$1"
    local source="$2"
    local relative_path="${destination#"${PURRVIEW_INTEGRATION_PREFIX}/"}"
    local backup_path="${PURRVIEW_INSTALL_ROOT}/integration-backup/${relative_path}"
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

purrview_link_integration() {
    mkdir -p "${PURRVIEW_INTEGRATION_PREFIX}/bin" \
        "${PURRVIEW_INTEGRATION_PREFIX}/share/applications" \
        "${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor/scalable/apps" \
        "${PURRVIEW_INTEGRATION_PREFIX}/share/kio/servicemenus" \
        "${PURRVIEW_INTEGRATION_PREFIX}/share/metainfo"

    local destination
    local source
    while IFS='|' read -r destination source; do
        purrview_link_one "${destination}" "${source}"
    done < <(purrview_integration_pairs)

    : >"${PURRVIEW_INSTALL_MANIFEST}"
    while IFS='|' read -r destination source; do
        local relative_path="${destination#"${PURRVIEW_INTEGRATION_PREFIX}/"}"
        local backup_path="integration-backup/${relative_path}"
        [[ -e "${PURRVIEW_INSTALL_ROOT}/${backup_path}" || -L "${PURRVIEW_INSTALL_ROOT}/${backup_path}" ]] || backup_path="-"
        printf '%s\t%s\t%s\n' "${destination}" "${source}" "${backup_path}" >>"${PURRVIEW_INSTALL_MANIFEST}"
    done < <(purrview_integration_pairs)
}

purrview_rewrite_manifest_backup() {
    local installed_path="$1"
    local backup_relative="$2"
    local temporary_manifest="${PURRVIEW_INSTALL_MANIFEST}.tmp.$$"
    local recorded_path
    local recorded_target
    local recorded_backup

    : >"${temporary_manifest}"
    while IFS=$'\t' read -r recorded_path recorded_target recorded_backup; do
        if [[ "${recorded_path}" == "${installed_path}" ]]; then
            recorded_backup="${backup_relative}"
        fi
        printf '%s\t%s\t%s\n' "${recorded_path}" "${recorded_target}" "${recorded_backup}" \
            >>"${temporary_manifest}"
    done <"${PURRVIEW_INSTALL_MANIFEST}"
    mv "${temporary_manifest}" "${PURRVIEW_INSTALL_MANIFEST}"
}

purrview_adopt_legacy_backups() {
    local installed_path
    local expected_target
    local legacy_backup_relative
    while IFS=$'\t' read -r installed_path expected_target legacy_backup_relative; do
        [[ -L "${installed_path}" ]] || continue
        [[ "$(readlink "${installed_path}")" == "${PURRVIEW_INSTALL_ROOT}"/* ]] || continue

        local relative_path="${installed_path#"${PURRVIEW_INTEGRATION_PREFIX}/"}"
        local new_backup_relative="integration-backup/${relative_path}"
        local new_backup_path="${PURRVIEW_INSTALL_ROOT}/${new_backup_relative}"
        rm -f -- "${new_backup_path}"
        if [[ "${legacy_backup_relative}" != "-" ]] && \
            [[ -e "${PURRVIEW_LEGACY_INSTALL_ROOT}/${legacy_backup_relative}" || \
               -L "${PURRVIEW_LEGACY_INSTALL_ROOT}/${legacy_backup_relative}" ]]; then
            mkdir -p "$(dirname "${new_backup_path}")"
            cp -a -- "${PURRVIEW_LEGACY_INSTALL_ROOT}/${legacy_backup_relative}" \
                "${new_backup_path}"
            purrview_rewrite_manifest_backup "${installed_path}" "${new_backup_relative}"
        else
            purrview_rewrite_manifest_backup "${installed_path}" "-"
        fi
    done <"${PURRVIEW_LEGACY_INSTALL_MANIFEST}"
}

purrview_remove_recorded_links() {
    local install_root="$1"
    local install_manifest="$2"
    local installed_path
    local expected_target
    local backup_relative

    while IFS=$'\t' read -r installed_path expected_target backup_relative; do
        case "${installed_path}" in
        "${PURRVIEW_INTEGRATION_PREFIX}"/*)
            local removed=0
            if [[ -L "${installed_path}" && "$(readlink "${installed_path}")" == "${expected_target}" ]]; then
                rm -f -- "${installed_path}"
                removed=1
            elif [[ -L "${installed_path}" && \
                    "$(readlink "${installed_path}")" == "${PURRVIEW_INSTALL_ROOT}"/* ]]; then
                continue
            elif [[ -e "${installed_path}" || -L "${installed_path}" ]]; then
                purrview_warn "Preserving externally changed path: ${installed_path}"
                continue
            else
                removed=1
            fi
            if ((removed)) && [[ "${backup_relative}" != "-" ]]; then
                local backup_path="${install_root}/${backup_relative}"
                if [[ -e "${backup_path}" || -L "${backup_path}" ]]; then
                    mkdir -p "$(dirname "${installed_path}")"
                    mv "${backup_path}" "${installed_path}"
                fi
            fi
            ;;
        *) purrview_die "Unsafe path in install manifest: ${installed_path}" ;;
        esac
    done <"${install_manifest}"
}

purrview_remove_install_root() {
    local install_root="$1"
    case "${install_root}" in
    "${HOME}/.local/opt/purrview" | "${HOME}/.local/opt/impage" | /opt/purrview | /opt/impage)
        rm -rf -- "${install_root}"
        ;;
    *) purrview_die "Refusing to remove unexpected root ${install_root}." ;;
    esac
}

purrview_cleanup_legacy_install() {
    [[ -f "${PURRVIEW_LEGACY_INSTALL_MANIFEST}" ]] || return 0
    purrview_adopt_legacy_backups
    purrview_remove_recorded_links \
        "${PURRVIEW_LEGACY_INSTALL_ROOT}" "${PURRVIEW_LEGACY_INSTALL_MANIFEST}"
    purrview_remove_install_root "${PURRVIEW_LEGACY_INSTALL_ROOT}"
    purrview_info "Migrated the legacy Impage bootstrap installation to PurrView."
}

purrview_activate_stage() {
    local backup_root="${PURRVIEW_INSTALL_ROOT}.backup.$$"
    if [[ -e "${PURRVIEW_INSTALL_ROOT}" ]]; then
        mv "${PURRVIEW_INSTALL_ROOT}" "${backup_root}"
    fi
    if ! mv "${PURRVIEW_STAGE_ROOT}" "${PURRVIEW_INSTALL_ROOT}"; then
        [[ ! -e "${backup_root}" ]] || mv "${backup_root}" "${PURRVIEW_INSTALL_ROOT}"
        purrview_die "Could not activate the staged installation."
    fi
    if [[ -d "${backup_root}/integration-backup" ]]; then
        cp -a -- "${backup_root}/integration-backup" "${PURRVIEW_INSTALL_ROOT}/"
    fi
    if ! purrview_link_integration || ! "${PURRVIEW_INSTALL_ROOT}/bin/purrview" --version; then
        rm -rf -- "${PURRVIEW_INSTALL_ROOT}"
        [[ ! -e "${backup_root}" ]] || mv "${backup_root}" "${PURRVIEW_INSTALL_ROOT}"
        purrview_die "Installed files failed validation; the previous version was restored."
    fi
    purrview_cleanup_legacy_install
    [[ ! -e "${backup_root}" ]] || rm -rf -- "${backup_root}"
    purrview_refresh_desktop
    purrview_ok "Installed at ${PURRVIEW_INSTALL_ROOT}"
    purrview_info "Launch with: ${PURRVIEW_INTEGRATION_PREFIX}/bin/purrview"
}

purrview_uninstall() {
    local install_root="${PURRVIEW_INSTALL_ROOT}"
    local install_manifest="${PURRVIEW_INSTALL_MANIFEST}"
    if [[ ! -f "${install_manifest}" && -f "${PURRVIEW_LEGACY_INSTALL_MANIFEST}" ]]; then
        install_root="${PURRVIEW_LEGACY_INSTALL_ROOT}"
        install_manifest="${PURRVIEW_LEGACY_INSTALL_MANIFEST}"
    fi
    [[ -f "${install_manifest}" ]] || \
        purrview_die "No PurrView bootstrap manifest exists at ${PURRVIEW_INSTALL_MANIFEST}."

    purrview_remove_recorded_links "${install_root}" "${install_manifest}"
    purrview_remove_install_root "${install_root}"
    purrview_refresh_desktop
    purrview_ok "PurrView bootstrap installation removed; user settings were preserved."
}
