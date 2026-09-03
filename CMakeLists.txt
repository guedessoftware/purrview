cmake_minimum_required(VERSION 3.21)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" IMPAGE_VERSION_RAW LIMIT_COUNT 1)
string(STRIP "${IMPAGE_VERSION_RAW}" IMPAGE_VERSION)
if(NOT IMPAGE_VERSION MATCHES
   "^[0-9]+\\.[0-9]+\\.[0-9]+(-[0-9A-Za-z.-]+)?(\\+[0-9A-Za-z.-]+)?$")
    message(FATAL_ERROR "VERSION must contain one valid Semantic Versioning value")
endif()
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" IMPAGE_VERSION_CORE "${IMPAGE_VERSION}")

project(PurrView VERSION "${IMPAGE_VERSION_CORE}" LANGUAGES CXX)

include(GNUInstallDirs)
include(cmake/DependencyVersions.cmake)
include(cmake/ReleaseMetadata.cmake)

configure_file(cmake/ImpageVersion.h.in generated/ImpageVersion.h @ONLY)
configure_file(resources/linux/io.github.impage.Impage.metainfo.xml.in
               generated/io.github.impage.Impage.metainfo.xml @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 ${IMPAGE_MIN_QT_VERSION} REQUIRED COMPONENTS
    Core Gui Network Qml Quick QuickControls2 PrintSupport Widgets)

set(IMPAGE_WITH_EXIV2 "AUTO" CACHE STRING
    "Advanced EXIF metadata: AUTO, ON (required), or OFF")
set_property(CACHE IMPAGE_WITH_EXIV2 PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${IMPAGE_WITH_EXIV2}" IMPAGE_WITH_EXIV2)
if(NOT IMPAGE_WITH_EXIV2 MATCHES "^(AUTO|ON|OFF)$")
    message(FATAL_ERROR "IMPAGE_WITH_EXIV2 must be AUTO, ON, or OFF")
endif()
set(IMPAGE_EXIV2_AVAILABLE FALSE)
if(NOT IMPAGE_WITH_EXIV2 STREQUAL "OFF")
    # Exiv2's config package currently uses ExactVersion compatibility, so find it first and
    # enforce Impage's minimum explicitly.
    find_package(exiv2 QUIET CONFIG)
    if(exiv2_FOUND AND exiv2_VERSION VERSION_LESS IMPAGE_MIN_EXIV2_VERSION)
        message(STATUS
            "Exiv2 ${exiv2_VERSION} is below the supported ${IMPAGE_MIN_EXIV2_VERSION}")
    elseif(TARGET Exiv2::exiv2lib)
        set(IMPAGE_EXIV2_AVAILABLE TRUE)
    endif()
endif()
if(IMPAGE_WITH_EXIV2 STREQUAL "ON" AND NOT IMPAGE_EXIV2_AVAILABLE)
    message(FATAL_ERROR
        "IMPAGE_WITH_EXIV2=ON requires Exiv2 >= ${IMPAGE_MIN_EXIV2_VERSION}")
endif()

qt_standard_project_setup(REQUIRES 6.4)

add_library(impage_core STATIC
    src/core/navigation/ComposerActivationContext.h
    src/core/image/ImageEntry.h
    src/core/image/FolderImageModel.cpp
    src/core/image/FolderImageModel.h
    src/core/image/ImageFormatSupport.cpp
    src/core/image/ImageFormatSupport.h
    src/core/image/ImageMetadata.cpp
    src/core/image/ImageMetadata.h
    src/core/image/ImageMetadataModel.cpp
    src/core/image/ImageMetadataModel.h
    src/core/image/ImageMetadataService.cpp
    src/core/image/ImageMetadataService.h
    src/core/image/Exiv2MetadataBackend.cpp
    src/core/image/Exiv2MetadataBackend.h
    src/core/image/ImageSession.cpp
    src/core/image/ImageSession.h
    src/core/image/ThumbnailCache.cpp
    src/core/image/ThumbnailCache.h
    src/core/document/DocumentModel.cpp
    src/core/document/DocumentModel.h
    src/core/document/GridLayout.cpp
    src/core/document/GridLayout.h
    src/core/document/ImageItem.h
    src/core/document/PageModel.cpp
    src/core/document/PageModel.h
    src/core/layout/ImagePlacementEngine.cpp
    src/core/layout/ImagePlacementEngine.h
    src/core/layout/LayoutEngine.cpp
    src/core/layout/LayoutEngine.h
    src/core/layout/PageLayout.h
    src/core/layout/Units.h
    src/core/render/PageRenderer.cpp
    src/core/render/PageRenderer.h
)
target_include_directories(impage_core PUBLIC src "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(impage_core PUBLIC Qt6::Core Qt6::Gui)
if(IMPAGE_EXIV2_AVAILABLE)
    target_link_libraries(impage_core PRIVATE Exiv2::exiv2lib)
    target_compile_definitions(impage_core PRIVATE IMPAGE_HAS_EXIV2=1)
    message(STATUS "PurrView advanced metadata: Exiv2 enabled")
else()
    message(STATUS "PurrView advanced metadata: basic Qt backend only")
endif()
target_compile_options(impage_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(impage_composer STATIC
    src/composer/ComposerController.cpp
    src/composer/ComposerController.h
    src/composer/ComposerSessionAdapter.cpp
    src/composer/ComposerSessionAdapter.h
    src/platform/printing/PrintService.cpp
    src/platform/printing/PrintService.h
)
target_include_directories(impage_composer PUBLIC src)
target_link_libraries(impage_composer PUBLIC
    impage_core
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::PrintSupport
    Qt6::Widgets
)
target_compile_options(impage_composer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(impage_viewer STATIC
    src/viewer/ViewerController.cpp
    src/viewer/ViewerController.h
    src/viewer/ViewerNavigationState.h
    src/viewer/ViewerState.cpp
    src/viewer/ViewerState.h
)
target_include_directories(impage_viewer PUBLIC src)
target_link_libraries(impage_viewer PUBLIC impage_core Qt6::Core Qt6::Gui)
target_compile_options(impage_viewer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(impage_shell STATIC
    src/platform/desktop/CommandLineParser.cpp
    src/platform/desktop/CommandLineParser.h
    src/platform/desktop/OpenRequest.cpp
    src/platform/desktop/OpenRequest.h
    src/platform/desktop/SingleInstanceService.cpp
    src/platform/desktop/SingleInstanceService.h
    src/shell/ApplicationContext.cpp
    src/shell/ApplicationContext.h
    src/shell/ApplicationController.cpp
    src/shell/ApplicationController.h
    src/shell/ModuleManager.cpp
    src/shell/ModuleManager.h
    src/shell/OpenRequestHandler.cpp
    src/shell/OpenRequestHandler.h
)
target_include_directories(impage_shell PUBLIC src)
target_link_libraries(impage_shell PUBLIC impage_composer impage_viewer Qt6::Core Qt6::Network)
target_compile_options(impage_shell PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

qt_add_executable(impage
    src/main.cpp
    src/ui/ApplicationTheme.cpp
    src/ui/ApplicationTheme.h
    src/ui/PagePreviewItem.cpp
    src/ui/PagePreviewItem.h
    src/ui/ThumbnailImageProvider.cpp
    src/ui/ThumbnailImageProvider.h
)

set_source_files_properties(qml/shell/ShellWindow.qml PROPERTIES
    QT_RESOURCE_ALIAS ShellWindow.qml)
set_source_files_properties(qml/shell/AboutDialog.qml PROPERTIES
    QT_RESOURCE_ALIAS AboutDialog.qml)
set_source_files_properties(qml/composer/ComposerPage.qml PROPERTIES
    QT_RESOURCE_ALIAS ComposerPage.qml)
set_source_files_properties(qml/components/MetricSpinBox.qml PROPERTIES
    QT_RESOURCE_ALIAS MetricSpinBox.qml)
set_source_files_properties(qml/components/PagePreview.qml PROPERTIES
    QT_RESOURCE_ALIAS PagePreview.qml)
set_source_files_properties(qml/components/PurrIcon.qml PROPERTIES
    QT_RESOURCE_ALIAS PurrIcon.qml)
set_source_files_properties(qml/components/IconLabel.qml PROPERTIES
    QT_RESOURCE_ALIAS IconLabel.qml)
set_source_files_properties(qml/panels/LayoutPanel.qml PROPERTIES
    QT_RESOURCE_ALIAS LayoutPanel.qml)
set_source_files_properties(qml/viewer/ViewerPage.qml PROPERTIES
    QT_RESOURCE_ALIAS ViewerPage.qml)
set_source_files_properties(qml/viewer/ImageCanvas.qml PROPERTIES
    QT_RESOURCE_ALIAS ImageCanvas.qml)
set_source_files_properties(qml/viewer/ViewerToolbar.qml PROPERTIES
    QT_RESOURCE_ALIAS ViewerToolbar.qml)
set_source_files_properties(qml/viewer/ViewerToolButton.qml PROPERTIES
    QT_RESOURCE_ALIAS ViewerToolButton.qml)
set_source_files_properties(qml/viewer/NavigationButton.qml PROPERTIES
    QT_RESOURCE_ALIAS NavigationButton.qml)
set_source_files_properties(qml/viewer/FilmStrip.qml PROPERTIES
    QT_RESOURCE_ALIAS FilmStrip.qml)
set_source_files_properties(qml/viewer/FilmStripItem.qml PROPERTIES
    QT_RESOURCE_ALIAS FilmStripItem.qml)
set_source_files_properties(qml/viewer/ImageInfoPanel.qml PROPERTIES
    QT_RESOURCE_ALIAS ImageInfoPanel.qml)
set_source_files_properties(qml/viewer/MetadataSection.qml PROPERTIES
    QT_RESOURCE_ALIAS MetadataSection.qml)
set_source_files_properties(qml/viewer/MetadataRow.qml PROPERTIES
    QT_RESOURCE_ALIAS MetadataRow.qml)
set_source_files_properties(qml/theme/UiTheme.qml PROPERTIES
    QT_RESOURCE_ALIAS UiTheme.qml
    QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/viewer/InlineNotice.qml PROPERTIES
    QT_RESOURCE_ALIAS InlineNotice.qml)
set_source_files_properties(resources/icons/purrview.svg PROPERTIES
    QT_RESOURCE_ALIAS assets/purrview.svg)
set_source_files_properties(resources/icons/hicolor/256x256/apps/purrview.png PROPERTIES
    QT_RESOURCE_ALIAS assets/purrview-window.png)

qt_add_qml_module(impage
    URI Impage
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/shell/ShellWindow.qml
        qml/shell/AboutDialog.qml
        qml/composer/ComposerPage.qml
        qml/components/MetricSpinBox.qml
        qml/components/PagePreview.qml
        qml/components/PurrIcon.qml
        qml/components/IconLabel.qml
        qml/panels/LayoutPanel.qml
        qml/viewer/ViewerPage.qml
        qml/viewer/ImageCanvas.qml
        qml/viewer/ViewerToolbar.qml
        qml/viewer/ViewerToolButton.qml
        qml/viewer/NavigationButton.qml
        qml/viewer/FilmStrip.qml
        qml/viewer/FilmStripItem.qml
        qml/viewer/ImageInfoPanel.qml
        qml/viewer/MetadataSection.qml
        qml/viewer/MetadataRow.qml
        qml/theme/UiTheme.qml
        qml/viewer/InlineNotice.qml
    RESOURCES
        resources/icons/purrview.svg
        resources/icons/hicolor/256x256/apps/purrview.png
)

target_include_directories(impage PRIVATE src src/ui)
target_link_libraries(impage PRIVATE
    impage_shell
    impage_composer
    impage_viewer
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::PrintSupport
    Qt6::Widgets
)
target_compile_options(impage PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

include(CTest)
if(BUILD_TESTING)
    add_executable(impage_core_tests
        tests/unit/CoreTests.cpp
    )
    target_link_libraries(impage_core_tests PRIVATE impage_core Qt6::Core Qt6::Gui)
    add_test(NAME impage_core_tests COMMAND impage_core_tests)

    add_executable(impage_session_tests
        tests/unit/ImageSessionTests.cpp
    )
    target_link_libraries(impage_session_tests PRIVATE impage_core Qt6::Core Qt6::Gui)
    add_test(NAME impage_session_tests COMMAND impage_session_tests)

    add_executable(impage_composer_tests
        tests/unit/ComposerControllerTests.cpp
    )
    target_link_libraries(impage_composer_tests PRIVATE impage_composer)
    add_test(NAME impage_composer_tests COMMAND impage_composer_tests)
    set_tests_properties(impage_composer_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(impage_shell_tests
        tests/unit/ShellTests.cpp
    )
    target_link_libraries(impage_shell_tests PRIVATE impage_shell)
    add_test(NAME impage_shell_tests COMMAND impage_shell_tests)
    set_tests_properties(impage_shell_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(impage_viewer_tests
        tests/unit/ViewerControllerTests.cpp
    )
    target_link_libraries(impage_viewer_tests PRIVATE impage_viewer)
    add_test(NAME impage_viewer_tests COMMAND impage_viewer_tests)

    add_executable(impage_folder_model_tests
        tests/unit/FolderImageModelTests.cpp
    )
    target_link_libraries(impage_folder_model_tests PRIVATE impage_core Qt6::Core Qt6::Gui)
    add_test(NAME impage_folder_model_tests COMMAND impage_folder_model_tests)

    add_executable(impage_thumbnail_cache_tests
        tests/unit/ThumbnailCacheTests.cpp
    )
    target_link_libraries(impage_thumbnail_cache_tests PRIVATE impage_core Qt6::Core Qt6::Gui)
    add_test(NAME impage_thumbnail_cache_tests COMMAND impage_thumbnail_cache_tests)

    add_executable(impage_metadata_tests
        tests/unit/ImageMetadataTests.cpp
    )
    target_link_libraries(impage_metadata_tests PRIVATE impage_core Qt6::Core Qt6::Gui)
    if(IMPAGE_EXIV2_AVAILABLE)
        target_link_libraries(impage_metadata_tests PRIVATE Exiv2::exiv2lib)
        target_compile_definitions(impage_metadata_tests PRIVATE IMPAGE_HAS_EXIV2=1)
    endif()
    add_test(NAME impage_metadata_tests COMMAND impage_metadata_tests)

    add_executable(impage_desktop_integration_tests
        tests/unit/DesktopIntegrationTests.cpp
    )
    target_link_libraries(impage_desktop_integration_tests PRIVATE impage_shell Qt6::Core Qt6::Network)
    add_test(NAME impage_desktop_integration_tests COMMAND impage_desktop_integration_tests)

    add_executable(impage_theme_tests
        tests/unit/ApplicationThemeTests.cpp
        src/ui/ApplicationTheme.cpp
        src/ui/ApplicationTheme.h
    )
    target_include_directories(impage_theme_tests PRIVATE src)
    target_link_libraries(impage_theme_tests PRIVATE Qt6::Gui)
    target_compile_options(impage_theme_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    )
    add_test(NAME impage_theme_tests COMMAND impage_theme_tests)

    find_program(IMPAGE_BASH_EXECUTABLE bash REQUIRED)
    add_test(NAME impage_bootstrap_tests
        COMMAND "${IMPAGE_BASH_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/bootstrap-tests.sh")
endif()

install(TARGETS impage RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS resources/bin/purrview
    DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/linux/io.github.impage.Impage.desktop
              resources/linux/io.github.impage.Impage.Viewer.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/icons/impage.svg resources/icons/purrview.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(DIRECTORY resources/icons/hicolor/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor)
install(PROGRAMS resources/linux/impage-servicemenu.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kio/servicemenus)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/io.github.impage.Impage.metainfo.xml"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES LICENSE README.md CHANGELOG.md VERSION THIRD_PARTY_NOTICES.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    FILES_MATCHING PATTERN "*.md")

include(cmake/Packaging.cmake)
