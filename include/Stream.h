//
// Created by hajk0 on 1/29/24.
//

#ifndef INTERNETRADIOSERVER_STREAM_H
#define INTERNETRADIOSERVER_STREAM_H

#include <vector>
#include <string>
#include <queue>
#include <netinet/in.h>
#include <mutex>
#include <thread>


using namespace std;

struct ClientInfo {
    int socket;
    sockaddr_in address;//
    int mainSocket;
};

class Stream {
private:
    int PORT = 12343;
    int chunkSize = 1024 * 1024; // 1 MB chunk size
    std::vector<ClientInfo> clients;
    std::queue<std::string> songsQueue;
    bool skipFlag = false;

public:
    int start(const char *ip);
    int streamSong(std::string songName);
    int end();
    int playQueue();
    int addToQueue(std::string songName);
    void skipSong();
    // void deleteClient(int clientSocket);
};


#endif //INTERNETRADIOSERVER_STREAM_H
