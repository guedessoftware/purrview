cmake_minimum_required(VERSION 3.21)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PURRVIEW_VERSION_RAW LIMIT_COUNT 1)
string(STRIP "${PURRVIEW_VERSION_RAW}" PURRVIEW_VERSION)
if(NOT PURRVIEW_VERSION MATCHES
   "^[0-9]+\\.[0-9]+\\.[0-9]+(-[0-9A-Za-z.-]+)?(\\+[0-9A-Za-z.-]+)?$")
    message(FATAL_ERROR "VERSION must contain one valid Semantic Versioning value")
endif()
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" PURRVIEW_VERSION_CORE "${PURRVIEW_VERSION}")

project(PurrView VERSION "${PURRVIEW_VERSION_CORE}" LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(CheckPIESupported)
include(cmake/DependencyVersions.cmake)
include(cmake/ReleaseMetadata.cmake)

configure_file(cmake/PurrViewVersion.h.in generated/PurrViewVersion.h @ONLY)
configure_file(resources/linux/io.github.guedessoftware.PurrView.metainfo.xml.in
               generated/io.github.guedessoftware.PurrView.metainfo.xml @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

option(PURRVIEW_ENABLE_HARDENING "Enable portable release binary hardening" ON)
if(PURRVIEW_ENABLE_HARDENING AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag("-fstack-protector-strong" PURRVIEW_HAS_STACK_PROTECTOR_STRONG)
    check_cxx_compiler_flag("-fstack-clash-protection" PURRVIEW_HAS_STACK_CLASH_PROTECTION)
    check_linker_flag(CXX "-Wl,-z,relro,-z,now" PURRVIEW_HAS_FULL_RELRO)
    check_pie_supported()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    if(PURRVIEW_HAS_STACK_PROTECTOR_STRONG)
        add_compile_options($<$<CONFIG:Release>:-fstack-protector-strong>)
    endif()
    if(PURRVIEW_HAS_STACK_CLASH_PROTECTION)
        add_compile_options($<$<CONFIG:Release>:-fstack-clash-protection>)
    endif()
    set(PURRVIEW_EXTERNAL_COMPILE_FLAGS
        "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} $ENV{CPPFLAGS} $ENV{CXXFLAGS}")
    if(NOT PURRVIEW_EXTERNAL_COMPILE_FLAGS MATCHES "_FORTIFY_SOURCE")
        add_compile_definitions($<$<CONFIG:Release>:_FORTIFY_SOURCE=2>)
    endif()
    if(PURRVIEW_HAS_FULL_RELRO)
        add_link_options($<$<CONFIG:Release>:-Wl,-z,relro,-z,now>)
    endif()
endif()

find_package(Qt6 ${PURRVIEW_MIN_QT_VERSION} REQUIRED COMPONENTS
    Core Gui Network Qml Quick QuickControls2 PrintSupport Widgets)

if(DEFINED IMPAGE_WITH_EXIV2 AND NOT DEFINED PURRVIEW_WITH_EXIV2)
    message(DEPRECATION
        "IMPAGE_WITH_EXIV2 is deprecated; use PURRVIEW_WITH_EXIV2 instead")
    set(PURRVIEW_WITH_EXIV2 "${IMPAGE_WITH_EXIV2}" CACHE STRING
        "Advanced EXIF metadata: AUTO, ON (required), or OFF")
endif()
set(PURRVIEW_WITH_EXIV2 "AUTO" CACHE STRING
    "Advanced EXIF metadata: AUTO, ON (required), or OFF")
set_property(CACHE PURRVIEW_WITH_EXIV2 PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${PURRVIEW_WITH_EXIV2}" PURRVIEW_WITH_EXIV2)
if(NOT PURRVIEW_WITH_EXIV2 MATCHES "^(AUTO|ON|OFF)$")
    message(FATAL_ERROR "PURRVIEW_WITH_EXIV2 must be AUTO, ON, or OFF")
endif()
set(PURRVIEW_EXIV2_AVAILABLE FALSE)
if(NOT PURRVIEW_WITH_EXIV2 STREQUAL "OFF")
    # Exiv2's config package currently uses ExactVersion compatibility, so find it first and
    # enforce PurrView's minimum explicitly.
    find_package(exiv2 QUIET CONFIG)
    if(exiv2_FOUND AND exiv2_VERSION VERSION_LESS PURRVIEW_MIN_EXIV2_VERSION)
        message(STATUS
            "Exiv2 ${exiv2_VERSION} is below the supported ${PURRVIEW_MIN_EXIV2_VERSION}")
    elseif(TARGET Exiv2::exiv2lib)
        set(PURRVIEW_EXIV2_AVAILABLE TRUE)
    endif()
endif()
if(PURRVIEW_WITH_EXIV2 STREQUAL "ON" AND NOT PURRVIEW_EXIV2_AVAILABLE)
    message(FATAL_ERROR
        "PURRVIEW_WITH_EXIV2=ON requires Exiv2 >= ${PURRVIEW_MIN_EXIV2_VERSION}")
endif()

qt_standard_project_setup(REQUIRES 6.4)

add_library(purrview_core STATIC
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
target_include_directories(purrview_core PUBLIC src "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(purrview_core PUBLIC Qt6::Core Qt6::Gui)
if(PURRVIEW_EXIV2_AVAILABLE)
    target_link_libraries(purrview_core PRIVATE Exiv2::exiv2lib)
    target_compile_definitions(purrview_core PRIVATE PURRVIEW_HAS_EXIV2=1)
    message(STATUS "PurrView advanced metadata: Exiv2 enabled")
else()
    message(STATUS "PurrView advanced metadata: basic Qt backend only")
endif()
target_compile_options(purrview_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(purrview_composer STATIC
    src/composer/ComposerController.cpp
    src/composer/ComposerController.h
    src/composer/ComposerSessionAdapter.cpp
    src/composer/ComposerSessionAdapter.h
    src/platform/printing/PrintService.cpp
    src/platform/printing/PrintService.h
)
target_include_directories(purrview_composer PUBLIC src)
target_link_libraries(purrview_composer PUBLIC
    purrview_core
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::PrintSupport
    Qt6::Widgets
)
target_compile_options(purrview_composer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(purrview_viewer STATIC
    src/viewer/ViewerController.cpp
    src/viewer/ViewerController.h
    src/viewer/ViewerNavigationState.h
    src/viewer/ViewerState.cpp
    src/viewer/ViewerState.h
)
target_include_directories(purrview_viewer PUBLIC src)
target_link_libraries(purrview_viewer PUBLIC purrview_core Qt6::Core Qt6::Gui)
target_compile_options(purrview_viewer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_library(purrview_shell STATIC
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
target_include_directories(purrview_shell PUBLIC src)
target_link_libraries(purrview_shell PUBLIC purrview_composer purrview_viewer Qt6::Core Qt6::Network)
target_compile_options(purrview_shell PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

qt_add_executable(purrview
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
set_source_files_properties(qml/components/PurrIconLabel.qml PROPERTIES
    QT_RESOURCE_ALIAS PurrIconLabel.qml)
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

qt_add_qml_module(purrview
    URI PurrView
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/shell/ShellWindow.qml
        qml/shell/AboutDialog.qml
        qml/composer/ComposerPage.qml
        qml/components/MetricSpinBox.qml
        qml/components/PagePreview.qml
        qml/components/PurrIcon.qml
        qml/components/PurrIconLabel.qml
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

target_include_directories(purrview PRIVATE src src/ui)
target_link_libraries(purrview PRIVATE
    purrview_shell
    purrview_composer
    purrview_viewer
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::PrintSupport
    Qt6::Widgets
)
target_compile_options(purrview PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

include(CTest)
if(BUILD_TESTING)
    add_executable(purrview_core_tests
        tests/unit/CoreTests.cpp
    )
    target_link_libraries(purrview_core_tests PRIVATE purrview_core Qt6::Core Qt6::Gui)
    add_test(NAME purrview_core_tests COMMAND purrview_core_tests)

    add_executable(purrview_session_tests
        tests/unit/ImageSessionTests.cpp
    )
    target_link_libraries(purrview_session_tests PRIVATE purrview_core Qt6::Core Qt6::Gui)
    add_test(NAME purrview_session_tests COMMAND purrview_session_tests)

    add_executable(purrview_composer_tests
        tests/unit/ComposerControllerTests.cpp
    )
    target_link_libraries(purrview_composer_tests PRIVATE purrview_composer)
    add_test(NAME purrview_composer_tests COMMAND purrview_composer_tests)
    set_tests_properties(purrview_composer_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(purrview_shell_tests
        tests/unit/ShellTests.cpp
    )
    target_link_libraries(purrview_shell_tests PRIVATE purrview_shell)
    add_test(NAME purrview_shell_tests COMMAND purrview_shell_tests)
    set_tests_properties(purrview_shell_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(purrview_viewer_tests
        tests/unit/ViewerControllerTests.cpp
    )
    target_link_libraries(purrview_viewer_tests PRIVATE purrview_viewer)
    add_test(NAME purrview_viewer_tests COMMAND purrview_viewer_tests)

    add_executable(purrview_folder_model_tests
        tests/unit/FolderImageModelTests.cpp
    )
    target_link_libraries(purrview_folder_model_tests PRIVATE purrview_core Qt6::Core Qt6::Gui)
    add_test(NAME purrview_folder_model_tests COMMAND purrview_folder_model_tests)

    add_executable(purrview_thumbnail_cache_tests
        tests/unit/ThumbnailCacheTests.cpp
    )
    target_link_libraries(purrview_thumbnail_cache_tests PRIVATE purrview_core Qt6::Core Qt6::Gui)
    add_test(NAME purrview_thumbnail_cache_tests COMMAND purrview_thumbnail_cache_tests)

    add_executable(purrview_metadata_tests
        tests/unit/ImageMetadataTests.cpp
    )
    target_link_libraries(purrview_metadata_tests PRIVATE purrview_core Qt6::Core Qt6::Gui)
    if(PURRVIEW_EXIV2_AVAILABLE)
        target_link_libraries(purrview_metadata_tests PRIVATE Exiv2::exiv2lib)
        target_compile_definitions(purrview_metadata_tests PRIVATE PURRVIEW_HAS_EXIV2=1)
    endif()
    add_test(NAME purrview_metadata_tests COMMAND purrview_metadata_tests)

    add_executable(purrview_desktop_integration_tests
        tests/unit/DesktopIntegrationTests.cpp
    )
    target_link_libraries(purrview_desktop_integration_tests PRIVATE purrview_shell Qt6::Core Qt6::Network)
    add_test(NAME purrview_desktop_integration_tests COMMAND purrview_desktop_integration_tests)

    add_executable(purrview_theme_tests
        tests/unit/ApplicationThemeTests.cpp
        src/ui/ApplicationTheme.cpp
        src/ui/ApplicationTheme.h
    )
    target_include_directories(purrview_theme_tests PRIVATE src)
    target_link_libraries(purrview_theme_tests PRIVATE Qt6::Gui)
    target_compile_options(purrview_theme_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    )
    add_test(NAME purrview_theme_tests COMMAND purrview_theme_tests)

    find_program(PURRVIEW_BASH_EXECUTABLE bash REQUIRED)
    add_test(NAME purrview_bootstrap_tests
        COMMAND "${PURRVIEW_BASH_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/bootstrap-tests.sh")
endif()

install(TARGETS purrview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# Temporary compatibility launcher for pre-PurrView scripts and desktop entries.
install(PROGRAMS resources/bin/impage
    DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/linux/io.github.guedessoftware.PurrView.desktop
              resources/linux/io.github.guedessoftware.PurrView.Viewer.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/icons/purrview.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(DIRECTORY resources/icons/hicolor/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor)
install(PROGRAMS resources/linux/purrview-servicemenu.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kio/servicemenus)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/io.github.guedessoftware.PurrView.metainfo.xml"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES LICENSE README.md CHANGELOG.md VERSION THIRD_PARTY_NOTICES.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    FILES_MATCHING PATTERN "*.md")

include(cmake/Packaging.cmake)
