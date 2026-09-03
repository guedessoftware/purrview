# Native binary package metadata shared by the DEB and RPM container builds.
# Packages deliberately depend on the target distribution's Qt runtime instead
# of bundling libraries from the build host.

set(CPACK_PACKAGE_NAME "purrview")
set(CPACK_PACKAGE_VENDOR "PurrView contributors")
set(CPACK_PACKAGE_CONTACT "PurrView contributors <guedessoftware@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Image viewer and printable page composer")
set(CPACK_PACKAGE_DESCRIPTION
    "PurrView combines a fast image viewer with a page composer for arranging and printing photos.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/guedessoftware/purrview")
set(CPACK_PACKAGE_VERSION "${IMPAGE_VERSION_CORE}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_STRIP_FILES ON)

set(CPACK_DEBIAN_PACKAGE_NAME "purrview")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-dialogs, qml6-module-qtquick-layouts, qml6-module-qtquick-templates, qml6-module-qtquick-window, qml6-module-qtqml, qml6-module-qtqml-models, qml6-module-qtqml-workerscript, libqt6svg6")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS
    "qt6-image-formats-plugins, qt6-wayland, shared-mime-info")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

set(CPACK_RPM_PACKAGE_NAME "purrview")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-only")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui, qt6-qtdeclarative, qt6-qtsvg")
set(CPACK_RPM_PACKAGE_SUGGESTS "qt6-qtimageformats, qt6-qtwayland")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    "/usr/share/applications"
    "/usr/share/doc"
    "/usr/share/icons"
    "/usr/share/icons/hicolor"
    "/usr/share/kio"
    "/usr/share/metainfo")

include(CPack)
