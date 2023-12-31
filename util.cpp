#include "util.h"

namespace util {
    static const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";


    static inline bool is_base64(char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) {
        std::string ret;
        int i = 0;
        int j = 0;
        char char_array_3[3];
        char char_array_4[4];

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
            }
        }

        if (i)
        {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while((i++ < 3))
                ret += '=';

        }

        return ret;
    }

    std::string base64_decode(std::string const& encoded_string) {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i ==4) {
                for (i = 0; i <4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                    i = 0;
            }
        }

        if (i) {
            for (j = i; j <4; j++)
            char_array_4[j] = 0;

            for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }

    std::string encodeFile(const char * filename) {

        std::ifstream infile(filename, std::ios::binary);
        std::istreambuf_iterator<char> it{infile};

        if (!infile) {
            fputs("File could not be opened", stderr);
            return "";
            //exit(1);
        }

        // get file size
        int size;
        infile.seekg(0,std::ios::end);
        size = infile.tellg();
        infile.seekg(0);

        char * decoded_chars = new char[size+1];

        infile.read(decoded_chars,size);

        std::string encoded_string = base64_encode(decoded_chars, size);

        delete[] decoded_chars;

        return encoded_string;
    }

    void log(const std::string &message) {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &error_message) {
        log("ERROR: " + error_message);
        exit(1);
    }

    std::string readFileToString(const char * file_name) {
        FILE *fp;
        unsigned long l_size;
        size_t result;
        char * buffer;
        
        fp = fopen(file_name,"rb");
        if (fp == NULL) {
            fputs("Could not load file",stderr);
            return "";
            //exit(1);
        }

        // obtain file size
        fseek(fp,0,SEEK_END);
        l_size = ftell(fp);
        rewind(fp);

        // allocate memory to contain the whole file
        buffer = (char *) malloc(sizeof(char)*l_size);
        if (buffer == NULL) {
            fputs("Failed to allocate memory for file",stderr);
            exit(1);
        }

        // copy the file into the buffer
        result = fread(buffer, 1, l_size, fp);
        if (result != l_size) {
            fputs("Failed to copy file into the buffer",stderr);
            exit(1);
        }

        std::string output(buffer, l_size);

        free(buffer);
        
        return output;
    }

    std::string setFileName(const std::string filepath) {
        int filepath_dot = filepath.find_last_of(".");
        std::string filepath_body = filepath.substr(0,filepath_dot);
        std::string extension = filepath.substr(filepath_dot);
        std::string unique_filepath = filepath;
        int name_increment = 0;

        std::regex re("(.*)\\(([0-9]+)\\)");
        if (std::regex_match(filepath_body, re)) {
            filepath_body = filepath_body.substr(0,filepath_body.find_last_of("("));
        }

        // check if file exists and if it does add 1 to the increment
        while (access( unique_filepath.c_str(), F_OK ) != -1 ) {
            name_increment++;
            unique_filepath = filepath_body + "(" + std::to_string(name_increment) + ")" + extension;
        }

        return unique_filepath;
    }

    std::string getPlaylistDirectory() {
        std::string AppSettings = readFileToString("../AppSettings/AppSettings.txt");
        int pos1 = AppSettings.find("PlaylistDirectory: ") + 19;
        int pos2 = AppSettings.find("\n",pos1);
        return AppSettings.substr(pos1,pos2-pos1);
    }

    int getPort() {
        std::string AppSettings = readFileToString("../AppSettings/AppSettings.txt");
        int pos1 = AppSettings.find("Port: ") + 6;
        int pos2 = AppSettings.find("\n",pos1);
        return std::stoi(AppSettings.substr(pos1,pos2-pos1));
    }
}