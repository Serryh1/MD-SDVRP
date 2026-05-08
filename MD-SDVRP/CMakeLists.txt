cmake_minimum_required(VERSION 3.5.0)
project(MDSDVRP VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB BLOSSOM_FILES "${CMAKE_SOURCE_DIR}/SRC/blossom/*.cpp")

set(MINCOST_FILE "${CMAKE_SOURCE_DIR}/SRC/blossom/MinCost/MinCost.cpp")

add_executable(MDSDVRP 
    SRC/MDSDVRP.cpp       
    ${BLOSSOM_FILES}     
    ${MINCOST_FILE}     
)
target_include_directories(MDSDVRP PUBLIC 
    "${CMAKE_SOURCE_DIR}/SRC/blossom"
    "${CMAKE_SOURCE_DIR}/SRC/blossom/MinCost"
)