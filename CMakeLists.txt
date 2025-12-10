cmake_minimum_required(VERSION 3.15)


project(game1 CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY game1/bin)


include(FetchContent)


include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    
)
FetchContent_MakeAvailable(raylib)

set(SOURCES main.cpp menu.cpp player.cpp platforms.cpp level1.cpp)


add_executable(game1 main.cpp menu.cpp player.cpp platforms.cpp level1.cpp)

target_link_libraries(game1 raylib)