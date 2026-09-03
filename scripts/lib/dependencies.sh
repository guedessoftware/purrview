#!/usr/bin/env bash

impage_dependency_packages() {
    case "${IMPAGE_DISTRO_FAMILY}" in
    ubuntu | debian)
        IMPAGE_CORE_PACKAGES=(build-essential cmake ninja-build pkg-config git xdg-utils
            qt6-base-dev qt6-declarative-dev qt6-image-formats-plugins libcups2-dev
            libxkbcommon-dev kimageformat6-plugins)
        IMPAGE_OPTIONAL_PACKAGES=(libexiv2-dev)
        ;;
    fedora)
        IMPAGE_CORE_PACKAGES=(gcc-c++ cmake ninja-build pkgconf-pkg-config git xdg-utils
            qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtimageformats cups-devel
            libxkbcommon-devel kf6-kimageformats libavif libheif)
        IMPAGE_OPTIONAL_PACKAGES=(exiv2-devel)
        ;;
    arch)
        IMPAGE_CORE_PACKAGES=(base-devel cmake ninja pkgconf git xdg-utils qt6-base qt6-declarative
            qt6-imageformats kimageformats libavif libheif cups libxkbcommon)
        IMPAGE_OPTIONAL_PACKAGES=(exiv2)
        ;;
    esac
}

impage_package_installed() {
    local package_name="$1"
    case "${IMPAGE_DISTRO_FAMILY}" in
    ubuntu | debian) dpkg-query -W -f='${Status}' "${package_name}" 2>/dev/null | grep -q 'install ok installed' ;;
    fedora) rpm -q "${package_name}" >/dev/null 2>&1 ;;
    arch) pacman -Q "${package_name}" >/dev/null 2>&1 ;;
    esac
}

impage_check_dependencies() {
    impage_dependency_packages
    IMPAGE_MISSING_PACKAGES=()
    local package_name
    for package_name in "${IMPAGE_CORE_PACKAGES[@]}"; do
        if ! impage_package_installed "${package_name}"; then
            IMPAGE_MISSING_PACKAGES+=("${package_name}")
        fi
    done

    local qt_minimum
    qt_minimum="$(impage_lock_value IMPAGE_MIN_QT_VERSION)"
    local qt_version=""
    if command -v pkg-config >/dev/null 2>&1; then
        qt_version="$(pkg-config --modversion Qt6Core 2>/dev/null || true)"
    fi
    if [[ -z "${qt_version}" ]] || ! impage_version_ge "${qt_version}" "${qt_minimum}"; then
        impage_warn "Qt ${qt_minimum}+ was not detected (found: ${qt_version:-none})."
        IMPAGE_DEPENDENCIES_OK=0
    elif ((${#IMPAGE_MISSING_PACKAGES[@]} > 0)); then
        IMPAGE_DEPENDENCIES_OK=0
    else
        IMPAGE_DEPENDENCIES_OK=1
    fi

    if ((IMPAGE_DEPENDENCIES_OK)); then
        impage_ok "Core toolchain and Qt ${qt_version}"
    else
        impage_warn "Missing core packages: ${IMPAGE_MISSING_PACKAGES[*]:-Qt ${qt_minimum}+}"
    fi

    if pkg-config --exists exiv2 2>/dev/null; then
        impage_ok "Exiv2 $(pkg-config --modversion exiv2) (advanced metadata)"
    else
        impage_warn "Exiv2 is optional; basic metadata will remain available."
    fi
}

impage_install_dependencies() {
    ((${#IMPAGE_MISSING_PACKAGES[@]} > 0)) || return 0
    impage_info "Required packages: ${IMPAGE_MISSING_PACKAGES[*]}"
    if ! impage_confirm "Install them with the distribution package manager?"; then
        impage_die "Dependencies were not installed."
    fi

    local elevate=()
    if ((EUID != 0)); then
        command -v sudo >/dev/null 2>&1 || impage_die "sudo is required to install system packages."
        elevate=(sudo)
    fi
    case "${IMPAGE_DISTRO_FAMILY}" in
    ubuntu | debian)
        "${elevate[@]}" apt-get update
        "${elevate[@]}" apt-get install -y "${IMPAGE_MISSING_PACKAGES[@]}"
        ;;
    fedora) "${elevate[@]}" dnf install -y "${IMPAGE_MISSING_PACKAGES[@]}" ;;
    arch) "${elevate[@]}" pacman -S --needed "${IMPAGE_MISSING_PACKAGES[@]}" ;;
    esac
}
