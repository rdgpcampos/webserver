#ifndef INCLUDED_CLIENT_HANDLER
#define INCLUDED_CLIENT_HANDLER

#include <string>
#include <unistd.h>
#include "util.h"
#include <regex>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>

namespace handler {
    typedef std::vector<std::string> (*buildResponse) (std::string); 
    const int BUFFER_SIZE = 30270;
    void * handle_client(void * client);

    std::vector<std::string> getMusicname(std::string request);
    std::vector<std::string> getMusics(std::string request);
    std::vector<std::string> get(std::string request);
    std::vector<std::string> postMusicname(std::string request);
    std::vector<std::string> getEmpty(std::string request);
    std::vector<std::string> getJS(std::string request);
}

#endif