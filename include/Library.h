//
// Created by hajk0 on 1/27/24.
//

#ifndef INTERNETRADIOSERVER_LIBRARY_H
#define INTERNETRADIOSERVER_LIBRARY_H

#include <string>
#include <filesystem>
#include <vector>

class Library {

public:
    std::string activeFileName;

    int addSong();
    std::vector<std::string> showSongs();
};


#endif //INTERNETRADIOSERVER_LIBRARY_H
