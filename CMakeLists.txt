project(opengles-imx6)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. DIR_SRCS)
add_executable(opengles-imx6 ${DIR_SRCS})

set (SRC_VERSION_MAJOR 1)
set (SRC_VERSION_MINOR 0)
set (SRC_VERSION_PATCH 1)
set (SRC_VERSION ${SRC_VERSION_MAJOR}.${SRC_VERSION_MINOR}.${SRC_VERSION_PATCH})
set (PROJECT_VERSION "${SRC_VERSION}")

add_definitions(-DLINUX)
add_definitions(-DEGL_API_FB)

target_link_libraries(opengles-imx6 EGL)
target_link_libraries(opengles-imx6 GLESv2)

install(TARGETS opengles-imx6 DESTINATION bin)
