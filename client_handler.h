#ifndef INCLUDED_CLIENT_HANDLER
#define INCLUDED_CLIENT_HANDLER

#include <string>
#include <unistd.h>
#include "util.h"
#include <regex>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include "http_tcpServer_linux.h"

namespace fs = std::__fs::filesystem;

namespace handler {
    typedef std::vector<std::string> (*buildResponse) (std::string); 
    void * handle_client(void * client);

    std::vector<std::string> getMusicname(std::string request);
    std::vector<std::string> getMusics(std::string request);
    std::vector<std::string> get(std::string request);
    std::vector<std::string> postMusicname(std::string request);
    std::vector<std::string> deleteMusicname(std::string request);
    std::vector<std::string> getEmpty(std::string request);
    std::vector<std::string> getJS(std::string request);
    std::vector<std::string> getFavicon(std::string request);
    std::vector<std::string> getImage(std::string request);
}

#endif