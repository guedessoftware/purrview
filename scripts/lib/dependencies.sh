#!/usr/bin/env bash

purrview_dependency_packages() {
    case "${PURRVIEW_DISTRO_FAMILY}" in
    ubuntu | debian)
        PURRVIEW_CORE_PACKAGES=(build-essential cmake ninja-build pkg-config git xdg-utils
            qt6-base-dev qt6-declarative-dev qt6-image-formats-plugins libcups2-dev
            libxkbcommon-dev kimageformat6-plugins)
        PURRVIEW_OPTIONAL_PACKAGES=(libexiv2-dev)
        ;;
    fedora)
        PURRVIEW_CORE_PACKAGES=(gcc-c++ cmake ninja-build pkgconf-pkg-config git xdg-utils
            qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtimageformats cups-devel
            libxkbcommon-devel kf6-kimageformats libavif libheif)
        PURRVIEW_OPTIONAL_PACKAGES=(exiv2-devel)
        ;;
    arch)
        PURRVIEW_CORE_PACKAGES=(base-devel cmake ninja pkgconf git xdg-utils qt6-base qt6-declarative
            qt6-imageformats kimageformats libavif libheif cups libxkbcommon)
        PURRVIEW_OPTIONAL_PACKAGES=(exiv2)
        ;;
    esac
}

purrview_package_installed() {
    local package_name="$1"
    case "${PURRVIEW_DISTRO_FAMILY}" in
    ubuntu | debian) dpkg-query -W -f='${Status}' "${package_name}" 2>/dev/null | grep -q 'install ok installed' ;;
    fedora) rpm -q "${package_name}" >/dev/null 2>&1 ;;
    arch) pacman -Q "${package_name}" >/dev/null 2>&1 ;;
    esac
}

purrview_check_dependencies() {
    purrview_dependency_packages
    PURRVIEW_MISSING_PACKAGES=()
    local package_name
    for package_name in "${PURRVIEW_CORE_PACKAGES[@]}"; do
        if ! purrview_package_installed "${package_name}"; then
            PURRVIEW_MISSING_PACKAGES+=("${package_name}")
        fi
    done

    local qt_minimum
    qt_minimum="$(purrview_lock_value PURRVIEW_MIN_QT_VERSION)"
    local qt_version=""
    if command -v pkg-config >/dev/null 2>&1; then
        qt_version="$(pkg-config --modversion Qt6Core 2>/dev/null || true)"
    fi
    if [[ -z "${qt_version}" ]] || ! purrview_version_ge "${qt_version}" "${qt_minimum}"; then
        purrview_warn "Qt ${qt_minimum}+ was not detected (found: ${qt_version:-none})."
        PURRVIEW_DEPENDENCIES_OK=0
    elif ((${#PURRVIEW_MISSING_PACKAGES[@]} > 0)); then
        PURRVIEW_DEPENDENCIES_OK=0
    else
        PURRVIEW_DEPENDENCIES_OK=1
    fi

    if ((PURRVIEW_DEPENDENCIES_OK)); then
        purrview_ok "Core toolchain and Qt ${qt_version}"
    else
        purrview_warn "Missing core packages: ${PURRVIEW_MISSING_PACKAGES[*]:-Qt ${qt_minimum}+}"
    fi

    if pkg-config --exists exiv2 2>/dev/null; then
        purrview_ok "Exiv2 $(pkg-config --modversion exiv2) (advanced metadata)"
    else
        purrview_warn "Exiv2 is optional; basic metadata will remain available."
    fi
}

purrview_install_dependencies() {
    ((${#PURRVIEW_MISSING_PACKAGES[@]} > 0)) || return 0
    purrview_info "Required packages: ${PURRVIEW_MISSING_PACKAGES[*]}"
    if ! purrview_confirm "Install them with the distribution package manager?"; then
        purrview_die "Dependencies were not installed."
    fi

    local elevate=()
    if ((EUID != 0)); then
        command -v sudo >/dev/null 2>&1 || purrview_die "sudo is required to install system packages."
        elevate=(sudo)
    fi
    case "${PURRVIEW_DISTRO_FAMILY}" in
    ubuntu | debian)
        "${elevate[@]}" apt-get update
        "${elevate[@]}" apt-get install -y "${PURRVIEW_MISSING_PACKAGES[@]}"
        ;;
    fedora) "${elevate[@]}" dnf install -y "${PURRVIEW_MISSING_PACKAGES[@]}" ;;
    arch) "${elevate[@]}" pacman -S --needed "${PURRVIEW_MISSING_PACKAGES[@]}" ;;
    esac
}
