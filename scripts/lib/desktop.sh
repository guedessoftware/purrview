#!/usr/bin/env bash

purrview_supported_mime_types() {
    printf '%s\n' image/png image/jpeg image/webp image/bmp image/gif image/tiff \
        image/avif image/heif image/heic image/x-icns
}

purrview_set_default_viewer() {
    command -v xdg-mime >/dev/null 2>&1 || {
        purrview_warn "xdg-mime is unavailable; default image associations were not changed."
        return 1
    }

    local desktop_id="io.github.guedessoftware.PurrView.Viewer.desktop"
    local mime_type
    local failed=0
    local command_output=""
    while IFS= read -r mime_type; do
        if ! command_output="$(xdg-mime default "${desktop_id}" "${mime_type}" 2>&1)"; then
            purrview_warn "Could not set the default Viewer for ${mime_type}: ${command_output}"
            failed=1
        fi
    done < <(purrview_supported_mime_types)

    ((failed == 0)) || return 1
    purrview_ok "PurrView is now the default for all supported image formats"
}

purrview_configure_desktop_launchers() {
    local executable="${PURRVIEW_INSTALL_ROOT}/bin/purrview"
    local icon="${PURRVIEW_INSTALL_ROOT}/share/icons/hicolor/256x256/apps/purrview.png"
    local escaped_executable="${executable//\\/\\\\}"
    escaped_executable="${escaped_executable//&/\\&}"
    escaped_executable="${escaped_executable//|/\\|}"
    local escaped_icon="${icon//\\/\\\\}"
    escaped_icon="${escaped_icon//&/\\&}"
    escaped_icon="${escaped_icon//|/\\|}"

    local main_desktop="${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.desktop"
    local viewer_desktop="${PURRVIEW_STAGE_ROOT}/share/applications/io.github.guedessoftware.PurrView.Viewer.desktop"
    local service_menu="${PURRVIEW_STAGE_ROOT}/share/kio/servicemenus/purrview-servicemenu.desktop"

    local desktop_file
    for desktop_file in "${main_desktop}" "${viewer_desktop}"; do
        [[ -f "${desktop_file}" ]] || purrview_die "Missing staged desktop launcher: ${desktop_file}"
        sed -i \
            -e "s|^Exec=purrview|Exec=${escaped_executable}|" \
            -e "s|^TryExec=purrview$|TryExec=${escaped_executable}|" \
            -e "s|^Icon=purrview$|Icon=${escaped_icon}|" \
            "${desktop_file}"
    done

    [[ -f "${service_menu}" ]] || purrview_die "Missing staged Dolphin service menu: ${service_menu}"
    sed -i \
        -e "s|^Exec=purrview|Exec=${escaped_executable}|" \
        -e "s|^Icon=purrview$|Icon=${escaped_icon}|" \
        "${service_menu}"

    grep -Fqx "Exec=${executable} %F" "${main_desktop}" || \
        purrview_die "The staged main launcher does not point to the installed executable."
    grep -Fqx "TryExec=${executable}" "${main_desktop}" || \
        purrview_die "The staged main launcher has an invalid TryExec path."
    grep -Fqx "Icon=${icon}" "${main_desktop}" || \
        purrview_die "The staged main launcher has an invalid icon path."
    grep -Fqx "Exec=${executable} --viewer %F" "${viewer_desktop}" || \
        purrview_die "The staged Viewer launcher does not point to the installed executable."
    grep -Fqx "TryExec=${executable}" "${viewer_desktop}" || \
        purrview_die "The staged Viewer launcher has an invalid TryExec path."
    grep -Fqx "Icon=${icon}" "${viewer_desktop}" || \
        purrview_die "The staged Viewer launcher has an invalid icon path."
    grep -Fqx "Exec=${executable} --compose %F" "${service_menu}" || \
        purrview_die "The staged Dolphin service menu does not point to the installed executable."
    grep -Fqx "Icon=${icon}" "${service_menu}" || \
        purrview_die "The staged Dolphin service menu has an invalid icon path."
}

purrview_refresh_desktop() {
    local applications_dir="${PURRVIEW_INTEGRATION_PREFIX}/share/applications"
    if command -v update-desktop-database >/dev/null 2>&1 && [[ -d "${applications_dir}" ]]; then
        update-desktop-database "${applications_dir}" || purrview_warn "Desktop database refresh failed."
    fi
    if command -v kbuildsycoca6 >/dev/null 2>&1; then
        kbuildsycoca6 || purrview_warn "KDE service cache refresh failed."
    fi
    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        gtk-update-icon-cache --force --ignore-theme-index \
            "${PURRVIEW_INTEGRATION_PREFIX}/share/icons/hicolor" >/dev/null || \
            purrview_warn "Icon cache refresh failed."
    fi
}
