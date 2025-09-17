cmake_minimum_required(VERSION 3.10)

project(QueueManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(tsunagi
    src/main.cpp
    src/queue_manager.cpp
    src/server.cpp
)

target_link_libraries(tsunagi PRIVATE ws2_32)
