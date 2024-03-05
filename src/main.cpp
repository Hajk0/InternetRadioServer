#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <netinet/in.h>
#include "../include/Server.h"

using namespace std;

int main() {
    Server server = Server();
    server.runServer();
    return 0;
}
