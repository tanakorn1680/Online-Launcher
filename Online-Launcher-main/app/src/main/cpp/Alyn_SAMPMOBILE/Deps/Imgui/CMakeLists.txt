cmake_minimum_required(VERSION 3.22.1)
project(imgui)

file(GLOB INCLUDE_FILES
        *.h
)

file(GLOB SOURCE_FILES
        *.cpp
)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE
        ${INCLUDE_FILES}
        ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}")
