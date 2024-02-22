//
// Created by hajk0 on 1/29/24.
//

#ifndef INTERNETRADIOSERVER_STREAM_H
#define INTERNETRADIOSERVER_STREAM_H

#include <vector>
#include <string>
#include <queue>
#include <netinet/in.h>

using namespace std;

struct ClientInfo {
    int socket;
    sockaddr_in address;
};

class Stream {
private:
    const int PORT = 12343;
    const int chunkSize = 1024 * 1024; // 1 MB chunk size
    vector<ClientInfo> clients;
    queue<string> songsQueue;

public:
    int start(const char *ip);
    int streamSong(string songName);
    int end();
    int playQueue();
    int addToQueue(string songName);
};


#endif //INTERNETRADIOSERVER_STREAM_H
