cmake_minimum_required(VERSION 3.27)
project(InternetRadioServer)

set(CMAKE_CXX_STANDARD 17)# 14

include_directories(/usr/include/SDL2)

find_library(SDL2_LIBRARY SDL2 /usr/include/SDL2)

add_executable(InternetRadioServer
        src/main.cpp
        src/Server.cpp
        include/TcpServer.h
        src/TcpServer.cpp
        include/UdpServer.h
        src/UdpServer.cpp
        include/Library.h
        src/Library.cpp
        include/Stream.h
        src/Stream.cpp
)

target_link_libraries(InternetRadioServer ${SDL2_LIBRARY})