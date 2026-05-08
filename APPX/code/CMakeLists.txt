cmake_minimum_required(VERSION 3.5.0)
project(APPX VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB BLOSSOM_FILES "${CMAKE_SOURCE_DIR}/src/blossom/*.cpp")

set(MINCOST_FILE "${CMAKE_SOURCE_DIR}/src/blossom/MinCost/MinCost.cpp")

add_executable(APPX
    src/main.cpp
    ${BLOSSOM_FILES} 
    ${MINCOST_FILE} 
)
target_include_directories(APPX PUBLIC
    "${CMAKE_SOURCE_DIR}/src/blossom"
    "${CMAKE_SOURCE_DIR}/src/blossom/MinCost"
)