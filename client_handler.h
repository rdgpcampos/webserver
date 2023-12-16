#ifndef INCLUDED_CLIENT_HANDLER
#define INCLUDED_CLIENT_HANDLER

#include <string>
#include <unistd.h>
#include "util.h"
#include <regex>
#include <sys/socket.h>
#include <unordered_map>

namespace handler {
    typedef std::string (*buildResponse) (std::string); 
    const int BUFFER_SIZE = 30270;
    void * handle_client(void * client);

    std::string getMusicname(std::string request);
    std::string getMusics(std::string request);
    std::string get(std::string request);
    std::string postMusicname(std::string request);
    std::string getEmpty(std::string request);
}

#endif