cmake_minimum_required(VERSION 3.31)
include(FetchContent)

project(infArmyDownloader LANGUAGES CXX)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git FIND_PACKAGE_ARGS)
FetchContent_MakeAvailable(cpr)

set(CPR_BUILD_TESTS OFF)

FetchContent_Declare(argparse GIT_REPOSITORY http://github.com/p-ranav/argparse.git)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cli GIT_REPOSITORY https://github.com/daniele77/cli.git)
FetchContent_MakeAvailable(cli)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(infArmyGet main.cpp)

target_link_libraries(infArmyGet PRIVATE cpr::cpr argparse nlohmann_json::nlohmann_json cli::cli)