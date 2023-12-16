#include <client_handler.h>

namespace fs = std::__fs::filesystem;


namespace handler {
	
	void * handle_client(void * socket) {
		char buffer[BUFFER_SIZE] = {0};
		const int bytes_received = read(*(int *)socket, buffer, BUFFER_SIZE); // receive request data from client and store into buffer
		const std::string request(buffer); // request as string
		std::string response;
        buildResponse builder;
        const std::unordered_map<std::string, buildResponse> response_function = {
            {"GET /?musicname",&getMusicname},
            {"GET /musics", &getMusics},
            {"GET /", &get},
            {"POST /?musicname", &postMusicname}
        };

        // test if content was read
        if (bytes_received < 0) {
			exitWithError("Failed to read bytes from client connection");
		}

        // find method and endpoint
        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string method = request.substr(0,pos1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);
        const std::string endpoint_parameter = endpoint.find("=") == -1? 
                                                endpoint.substr(0,endpoint.find_last_of(" ")) : endpoint.substr(0,endpoint.find("="));

        // build response using appropriate function
        if (response_function.find(method+" "+endpoint_parameter) != response_function.end()) {
                builder  = response_function.at(method+" "+endpoint_parameter);
                response = (*builder)(request);
        } else {
                response = getEmpty(request);
        }

		//send response
		char * response_array = new char[response.size()];
		strcpy(response_array, response.c_str());
		send(*(int *)socket,response_array, strlen(response_array),0);

		delete[] response_array;
		close(*(int *)socket);

		return NULL;
	}

    std::string getMusicname(std::string request) {
        std::string header;
		std::string body;

        int pos1 = request.find("musicname");
		int pos2 = request.find("HTTP");

        std::string filename_string = "../playlist/" + request.substr(pos1+10,pos2-pos1-11);

        char * filename = &filename_string[0];

        // build body
        body = encodeFile(filename); // convert to base64
        body = "{\"data\": \"" + body + "\"}";

        // build header
        header = "HTTP/1.1 200 OK /audiofile.mp3\r\n"
                        "Content-Type: application/json\r\n"
                        "Accept: */*\r\n"
                        "Connection: close\r\n"
                        "\r\n";

        return header + body;
    }

    std::string getMusics(std::string request) {
        std::string header;
		std::string body;

        // build header
        header = "HTTP/1.1 200 OK \r\n"
            "Content-Type: application/json\r\n"
            "Accept: */*\r\n"
            "Connection: close\r\n"
            "Content-Length: ";

        // prepare body containing playlist
        body = "[";
        std::string playlist_path = "../playlist";
        for (const auto & music : fs::directory_iterator(playlist_path)) {
            body = body + 
                        "{\"name\":\"" +
                        music.path().string().substr(playlist_path.length()+1) +
                        "\"},";
        }
        body = body.substr(0,body.length()-1) + "]";

        header = header + std::to_string(body.length()) + "\r\n\r\n";


        return header + body;
    }

    std::string get(std::string request) {
        std::string header;
		std::string body;

        // get initial page
        std::string html_file = readFileToString("../html/initial_page.html");

        // build response
        header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                std::to_string(html_file.size()) + "\n\n";
        body = html_file;

        return header + body;
    }

    std::string getEmpty(std::string request) {
        std::string header;
		std::string body;

        body = "NULL";

        // build response
        header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                std::to_string(body.size()) + "\n\n";

        return header + body;
    }

    std::string postMusicname(std::string request) {
        std::string header;
		std::string body;

        return header + body;
    }
}