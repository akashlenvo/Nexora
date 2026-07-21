cmake_minimum_required(VERSION 3.20)
project(Nexora VERSION 1.1.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This CMake configuration currently targets Linux. Use Nexora.sln on Windows.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
    libswscale
    libswresample
)

set(NEXORA_SOURCES
    3rdparty/qrcodegen/qrcodegen.cpp
    src/application.cpp
    src/canvas.cpp
    src/main.cpp
    src/settings.cpp
    src/snapshotmanager.cpp
    src/virtualcamera.cpp
    src/v4l2source.cpp
    src/gui/devicesview.cpp
    src/gui/imgadjdlg.cpp
    src/gui/qrconview.cpp
    src/gui/streamconfigdlg.cpp
    src/gui/window.cpp
    src/net/connection.cpp
    src/net/serializer.cpp
    src/net/server.cpp
    src/rtsp/manager.cpp
    src/rtsp/receiver.cpp
    src/video/1_1scaler.cpp
    src/video/directshowscaler.cpp
    src/video/framescaler.cpp
    src/video/guipreviewscaler.cpp
)

add_executable(Nexora ${NEXORA_SOURCES})
target_include_directories(Nexora PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qrcodegen
)
target_compile_definitions(Nexora PRIVATE ASIO_STANDALONE)
target_compile_options(Nexora PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(Nexora PRIVATE
    ${wxWidgets_LIBRARIES}
    PkgConfig::FFMPEG
    Threads::Threads
)

install(TARGETS Nexora RUNTIME DESTINATION .)
install(DIRECTORY res/ DESTINATION res
    FILES_MATCHING
    PATTERN "*.png"
    PATTERN "*.ico"
)
