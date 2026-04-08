
cmake_minimum_required(VERSION 3.16)
project(ColorPicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/app.cpp
    src/colorsampler.cpp
    src/trayicon.cpp
)

set(HEADERS
    src/app.h
    src/colorsampler.h
    src/trayicon.h
    src/resource.h
)

add_executable(ColorPicker WIN32
    ${SOURCES}
    ${HEADERS}
    res/app.rc
)

target_include_directories(ColorPicker PRIVATE src)

target_compile_definitions(ColorPicker PRIVATE UNICODE _UNICODE)

target_link_libraries(ColorPicker PRIVATE
    user32
    gdi32
    shell32
)

if(MSVC)
    target_compile_options(ColorPicker PRIVATE /W4 /O2)
endif()
