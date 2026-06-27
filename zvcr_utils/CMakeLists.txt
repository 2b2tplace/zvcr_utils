set(CMAKE_CXX_STANDARD 23)
set(BOTCRAFT_GAME_VERSION "1.21.4")
set(BOTCRAFT_INSTALL_MC_ASSETS TRUE)

find_package(PNG REQUIRED)
find_path(WEBP_INCLUDE_DIR webp/decode.h)
find_library(WEBP_LIBRARIES NAMES webp)

file(GLOB_RECURSE LIBS "src/*.hpp" "src/*.h" "src/*.cpp" "src/*.c")
include(FetchContent)

FetchContent_Declare(zvcr
        GIT_REPOSITORY git@github.com:2b2tplace/zvcr.git
        GIT_TAG        1.0.0.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(zvcr)

FetchContent_Declare(mc_cpp
        GIT_REPOSITORY git@github.com:2b2tplace/mc-cpp.git
        GIT_TAG        0.1.13.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(mc_cpp)

add_library(zvcr_utils ${LIBS})
target_include_directories(zvcr_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(zvcr_utils
        PUBLIC PNG::PNG
        PUBLIC ${WEBP_LIBRARIES}
        PUBLIC mc_cpp
        PUBLIC zvcr_lib
)
target_include_directories(zvcr_utils
        PUBLIC ${argh_INCLUDE_DIR}
        PUBLIC ${WEBP_INCLUDE_DIR}
)