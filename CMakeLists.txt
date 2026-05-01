cmake_minimum_required(VERSION 3.16)
project(RobotWarehouse)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(RobotWarehouse
    main.cpp
    world.cpp
    pathfinding.cpp
    worker.cpp
)

target_link_libraries(RobotWarehouse PRIVATE SFML::Graphics SFML::Window SFML::System)