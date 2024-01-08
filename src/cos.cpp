#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

using namespace std;

int main(int argc, char **argv) {
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sa, ca;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = PF_INET;
    sa.sin_port = htons(3000);
    sa.sin_addr.s_addr = INADDR_ANY;

    const int one = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    bind(server, (struct sockaddr*) &sa, sizeof(sa));

    listen(server, 1);

    socklen_t sl = sizeof(ca);
    int client = accept(server, (struct sockaddr*) &ca, &sl);


    int eFd = epoll_create1(0);

    epoll_event event;
    event.events = EPOLLIN;
    event.data.u64 = 987654321;
    epoll_ctl(eFd, EPOLL_CTL_ADD, client, &event);
    //event.data.u64 = 123456789;
    //epoll_ctl(eFd, EPOLL_CTL_ADD, )
    char buf[64];

    while(true) {
        int selector = epoll_wait(eFd, &event, 1, -1);

        if(event.events & EPOLLIN && event.data.u64 == 987654321) {
            read(client, buf, sizeof(buf));
        }
    }

    write(client, "siemano", 7);

    shutdown(server, SHUT_RDWR);
    shutdown(client, SHUT_RDWR);
    close(server);
    close(client);

    return 0;
}
