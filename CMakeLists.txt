cmake_minimum_required(VERSION 3.7)
project(course)

add_compile_options(-Wall)

set(APP_NAME program)

file(GLOB SOURCES src/*.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

add_definitions(${GTK4_CFLAGS_OTHER})

add_executable(${APP_NAME} ${SOURCES})

target_link_libraries(${APP_NAME} ${GTK4_LIBRARIES})
target_link_libraries(${APP_NAME} adwaita-1)
