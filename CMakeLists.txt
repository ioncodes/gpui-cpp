cmake_minimum_required(VERSION 3.22)

project(gpui-cpp
    VERSION 0.1.0
    DESCRIPTION "C++ bindings for GPUI"
    LANGUAGES C CXX)

option(GPUI_CPP_BUILD_EXAMPLES "Build gpui-cpp examples" ${PROJECT_IS_TOP_LEVEL})
option(GPUI_CPP_INSTALL "Generate install rules for gpui-cpp" ${PROJECT_IS_TOP_LEVEL})

if(NOT COMMAND corrosion_import_crate)
    include(FetchContent)
    FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG        v0.5.2
    )
    FetchContent_MakeAvailable(Corrosion)
endif()

corrosion_import_crate(
    MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/crates/gpui-ffi/Cargo.toml"
    CRATE_TYPES   staticlib
)

add_library(gpui_cpp INTERFACE)
add_library(gpui::cpp ALIAS gpui_cpp)

target_compile_features(gpui_cpp INTERFACE cxx_std_17)

target_include_directories(gpui_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(gpui_cpp INTERFACE $<BUILD_INTERFACE:gpui_ffi>)

if(APPLE)
    target_link_libraries(gpui_cpp INTERFACE
        "-framework AppKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreServices"
        "-framework CoreText"
        "-framework CoreVideo"
        "-framework Foundation"
        "-framework IOKit"
        "-framework IOSurface"
        "-framework Metal"
        "-framework QuartzCore"
        "-framework Security"
        "-framework UserNotifications"
        "-framework UniformTypeIdentifiers"
    )
elseif(WIN32)
    target_link_libraries(gpui_cpp INTERFACE
        user32 gdi32 ole32 oleaut32 shell32 shlwapi shcore advapi32 userenv
        ws2_32 bcrypt ntdll d3d11 dxgi dcomp dwmapi winmm imm32 uxtheme
        propsys runtimeobject
    )
else()
    find_package(Threads REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XKB REQUIRED IMPORTED_TARGET xkbcommon xkbcommon-x11 xcb)
    target_link_libraries(gpui_cpp INTERFACE
        Threads::Threads ${CMAKE_DL_LIBS} m
        Freetype::Freetype Fontconfig::Fontconfig PkgConfig::XKB)
endif()

if(GPUI_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(GPUI_CPP_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(TARGETS gpui_cpp EXPORT gpui-cpp-targets)
    install(EXPORT gpui-cpp-targets
        NAMESPACE gpui::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpui-cpp)

    install(FILES $<TARGET_FILE:gpui_ffi-static>
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        OPTIONAL)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/gpui-cpp-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gpui-cpp-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/gpui-cpp-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpui-cpp)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/gpui-cpp-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/gpui-cpp-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpui-cpp)
endif()
