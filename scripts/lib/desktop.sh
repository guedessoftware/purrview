#!/usr/bin/env bash

impage_supported_mime_types() {
    printf '%s\n' image/png image/jpeg image/webp image/bmp image/gif image/tiff \
        image/avif image/heif image/heic image/x-icns
}

impage_set_default_viewer() {
    command -v xdg-mime >/dev/null 2>&1 || {
        impage_warn "xdg-mime is unavailable; default image associations were not changed."
        return 1
    }

    local desktop_id="io.github.impage.Impage.Viewer.desktop"
    local mime_type
    local failed=0
    local command_output=""
    while IFS= read -r mime_type; do
        if ! command_output="$(xdg-mime default "${desktop_id}" "${mime_type}" 2>&1)"; then
            impage_warn "Could not set the default Viewer for ${mime_type}: ${command_output}"
            failed=1
        fi
    done < <(impage_supported_mime_types)

    ((failed == 0)) || return 1
    impage_ok "PurrView is now the default for all supported image formats"
}

impage_configure_desktop_launchers() {
    local executable="${IMPAGE_INSTALL_ROOT}/bin/purrview"
    local icon="${IMPAGE_INSTALL_ROOT}/share/icons/hicolor/256x256/apps/purrview.png"
    local escaped_executable="${executable//\\/\\\\}"
    escaped_executable="${escaped_executable//&/\\&}"
    escaped_executable="${escaped_executable//|/\\|}"
    local escaped_icon="${icon//\\/\\\\}"
    escaped_icon="${escaped_icon//&/\\&}"
    escaped_icon="${escaped_icon//|/\\|}"

    local main_desktop="${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.desktop"
    local viewer_desktop="${IMPAGE_STAGE_ROOT}/share/applications/io.github.impage.Impage.Viewer.desktop"
    local service_menu="${IMPAGE_STAGE_ROOT}/share/kio/servicemenus/impage-servicemenu.desktop"

    local desktop_file
    for desktop_file in "${main_desktop}" "${viewer_desktop}"; do
        [[ -f "${desktop_file}" ]] || impage_die "Missing staged desktop launcher: ${desktop_file}"
        sed -i \
            -e "s|^Exec=purrview|Exec=${escaped_executable}|" \
            -e "s|^TryExec=purrview$|TryExec=${escaped_executable}|" \
            -e "s|^Icon=purrview$|Icon=${escaped_icon}|" \
            "${desktop_file}"
    done

    [[ -f "${service_menu}" ]] || impage_die "Missing staged Dolphin service menu: ${service_menu}"
    sed -i \
        -e "s|^Exec=purrview|Exec=${escaped_executable}|" \
        -e "s|^Icon=purrview$|Icon=${escaped_icon}|" \
        "${service_menu}"

    grep -Fqx "Exec=${executable} %F" "${main_desktop}" || \
        impage_die "The staged main launcher does not point to the installed executable."
    grep -Fqx "TryExec=${executable}" "${main_desktop}" || \
        impage_die "The staged main launcher has an invalid TryExec path."
    grep -Fqx "Icon=${icon}" "${main_desktop}" || \
        impage_die "The staged main launcher has an invalid icon path."
    grep -Fqx "Exec=${executable} --viewer %F" "${viewer_desktop}" || \
        impage_die "The staged Viewer launcher does not point to the installed executable."
    grep -Fqx "TryExec=${executable}" "${viewer_desktop}" || \
        impage_die "The staged Viewer launcher has an invalid TryExec path."
    grep -Fqx "Icon=${icon}" "${viewer_desktop}" || \
        impage_die "The staged Viewer launcher has an invalid icon path."
    grep -Fqx "Exec=${executable} --compose %F" "${service_menu}" || \
        impage_die "The staged Dolphin service menu does not point to the installed executable."
    grep -Fqx "Icon=${icon}" "${service_menu}" || \
        impage_die "The staged Dolphin service menu has an invalid icon path."
}

impage_refresh_desktop() {
    local applications_dir="${IMPAGE_INTEGRATION_PREFIX}/share/applications"
    if command -v update-desktop-database >/dev/null 2>&1 && [[ -d "${applications_dir}" ]]; then
        update-desktop-database "${applications_dir}" || impage_warn "Desktop database refresh failed."
    fi
    if command -v kbuildsycoca6 >/dev/null 2>&1; then
        kbuildsycoca6 || impage_warn "KDE service cache refresh failed."
    fi
    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        gtk-update-icon-cache --force --ignore-theme-index \
            "${IMPAGE_INTEGRATION_PREFIX}/share/icons/hicolor" >/dev/null || \
            impage_warn "Icon cache refresh failed."
    fi
}
