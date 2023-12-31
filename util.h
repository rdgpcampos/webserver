#ifndef INCLUDED_UTIL
#define INCLUDED_UTIL

#include<iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <initializer_list>
#include <unordered_map>
#include <unistd.h>
#include <regex>

namespace util {
    const int BUFFER_SIZE = 30000;
    static inline bool is_base64(char c);
    std::string base64_encode(char const* bytes_to_encode, unsigned int in_len);
    std::string base64_decode(std::string const& encoded_string);
    std::string encodeFile(const char * filename);
    std::string readFileAsBinary(const char * filename);
    void log(const std::string &message);
    void exitWithError(const std::string &error_message);
    std::string readFileToString(const char * file_name);
    std::string setFileName(const std::string filepath);
    std::string getPlaylistDirectory();
    int getPort();
}

#endif