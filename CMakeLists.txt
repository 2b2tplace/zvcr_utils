cmake_minimum_required(VERSION 3.25)
project(zvcr_utils)

set(CMAKE_CXX_STANDARD 23)

include_directories(zvcr_cli zvcr_utils)
add_subdirectory(zvcr_utils)

add_executable(zvcr_cli main.cpp)
set_target_properties(zvcr_cli PROPERTIES OUTPUT_NAME "zvcr")
target_link_libraries(zvcr_cli zvcr_utils)