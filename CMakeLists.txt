cmake_minimum_required(VERSION 3.16)
project(WBoard VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 required modules
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Core5Compat
    Gui
    Widgets
    Network
    Xml
    OpenGL
    OpenGLWidgets
    PrintSupport
    Svg
    SvgWidgets
    Multimedia
    MultimediaWidgets
    WebSockets
    WebEngineWidgets
    WebEngineCore
    WebChannel
    Qml
    UiTools
)

# Quazip - bundled source or system
# The project ships quazip headers/libs; we use system quazip on Linux
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(QUAZIP quazip1-qt6 QUIET)
endif()
if(NOT QUAZIP_FOUND)
    pkg_check_modules(QUAZIP quazip-qt5 QUIET)
endif()
if(NOT QUAZIP_FOUND)
    message(WARNING "quazip not found via pkg-config; trying find_package")
    find_package(quazip QUIET)
endif()

# OpenSSL (for network)
find_package(OpenSSL REQUIRED)

# ZLIB (pdf-merger needs it)
find_package(ZLIB REQUIRED)

# Source directories
set(WBOARD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/WBoard/Source)
set(WBOARD_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/WBoard/UI)
set(WBOARD_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/WBoard/Resources)

# Tell AUTOUIC where to find .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS ${WBOARD_UI_DIR} ${WBOARD_SOURCE_DIR}/web/browser)

# Collect all source files
file(GLOB_RECURSE WBOARD_SOURCES
    ${WBOARD_SOURCE_DIR}/*.cpp
    ${WBOARD_SOURCE_DIR}/*.h
)
file(GLOB_RECURSE WBOARD_UI_FILES
    ${WBOARD_UI_DIR}/*.ui
    ${WBOARD_SOURCE_DIR}/web/browser/*.ui
)

# Exclude Windows-only files
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*_win\\.(cpp|h)$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*windowsmedia/.*")
# Exclude Windows lock files, use qtlockedfile_unix.cpp directly (included by qtlocalpeer.cpp)
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*qtlockedfile_win\\.cpp$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*qtsingleapplication/qtlockedfile\\.cpp$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*qtsingleapplication/qtlockedfile_unix\\.cpp$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*WBWindowCaptureDelegate_win.*")

# Exclude UB*/QtWebKit legacy files (Qt4/5, removed in Qt6)
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*network/UB.*\\.(cpp|h)$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*pdf/UB.*\\.(cpp|h)$")

# Exclude web browser subsystem (depends on excluded UB network files)
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*web/browser/.*\\.(cpp|h)$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*web/WBWebController\\.(cpp|h)$")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*podcast/ffmpeg/.*")
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*podcast/WBPodcastController\\.(cpp|h)$")

# Also exclude .rc files
list(FILTER WBOARD_SOURCES EXCLUDE REGEX ".*\\.rc$")

# Resources
set(WBOARD_QRC ${WBOARD_RESOURCES_DIR}/WBoard.qrc)

# Platform-specific sources
if(WIN32)
    file(GLOB WBOARD_PLATFORM_SOURCES
        ${WBOARD_SOURCE_DIR}/desktop/WBWindowCapture_win.cpp
        ${WBOARD_SOURCE_DIR}/desktop/WBWindowCaptureDelegate_win.cpp
        ${WBOARD_SOURCE_DIR}/frameworks/WBPlatformUtils_win.cpp
        ${WBOARD_SOURCE_DIR}/gui/WBKeyboardPalette_win.cpp
        ${WBOARD_SOURCE_DIR}/podcast/windowsmedia/*.cpp
    )
    list(APPEND WBOARD_SOURCES ${WBOARD_PLATFORM_SOURCES})
endif()

# Create executable
qt_add_executable(${PROJECT_NAME}
    ${WBOARD_SOURCES}
    ${WBOARD_UI_FILES}
    ${WBOARD_QRC}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${WBOARD_SOURCE_DIR}
    ${WBOARD_SOURCE_DIR}/web/browser
    ${WBOARD_UI_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}  # for generated ui_*.h
)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Core5Compat
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::WebSockets
    Qt6::WebEngineWidgets
    Qt6::WebEngineCore
    Qt6::WebChannel
    Qt6::Qml
    Qt6::UiTools
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

# Link quazip if found
if(QUAZIP_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${QUAZIP_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${QUAZIP_INCLUDE_DIRS})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm wmvcore)
endif()

# Platform defines
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE _UNICODE WIN32 WIN64
        _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)
endif()

# Suppress some warnings from Qt6 deprecated API usage in Core5Compat
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
)

message(STATUS "WBoard configured with Qt ${Qt6_VERSION}")
