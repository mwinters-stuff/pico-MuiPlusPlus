# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(GettingStarted C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


include(../libraries.cmake)
includeLibraries()

# Add executable. Default name is the project name, version 0.1

add_executable(GettingStarted src/main.cpp
    ../../src/muiplusplus.cpp
    ../../src/muipp_u8g2.cpp
    ../libs/fonts/u8g2_font_bauhaus2015_tr.c
    ../libs/fonts/u8g2_font_glasstown_nbp_t_all.c
    ../libs/u8g2pico/u8g2functions.cpp
)

pico_set_program_name(GettingStarted "GettingStarted")
pico_set_program_version(GettingStarted "0.1")


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(GettingStarted 1)
pico_enable_stdio_usb(GettingStarted 0)

# Add the subdirectory for the Versatile_RotaryEncoder library
# add_subdirectory(cmake)

# Add the standard library to the build
target_link_libraries(GettingStarted
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(GettingStarted PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ../../src
        ../libs/u8g2pico
)

# Add any user requested libraries
target_link_libraries(GettingStarted 
        Versatile_RotaryEncoder
        pico_stdlib
        u8g2
        hardware_i2c
        )

pico_add_extra_outputs(GettingStarted)

