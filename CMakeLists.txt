cmake_minimum_required(VERSION 3.27)
project(InternetRadioServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(InternetRadioServer
        src/main.cpp
        src/Server.cpp
        include/TcpServer.h
        src/TcpServer.cpp
        include/UdpServer.h
        src/UdpServer.cpp)
