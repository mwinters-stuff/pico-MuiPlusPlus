cmake_minimum_required(VERSION 3.28)

add_library(MuiPlusPlus
        ../src/muiplusplus.cpp
        ../src/muipp_u8g2.cpp
        ../src/muipp_tlp.cpp
)

target_compile_definitions(MuiPlusPlus
        PUBLIC BUILD_FOR_PICO_CMAKE=1
)

target_include_directories(MuiPlusPlus PUBLIC
        ../src
)

target_link_libraries(MuiPlusPlus PUBLIC
        pico_stdlib
        u8g2)