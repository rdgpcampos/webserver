#include <client_handler.h>

namespace fs = std::__fs::filesystem;

namespace handler {
	void * handle_client(void * socket) {
        std::vector<std::string> response;
        buildResponse builder;
        std::string request = getRequestAsString(socket, BUFFER_SIZE);
        const std::unordered_map<std::string, buildResponse> response_function = {
            {"GET /?musicname",&getMusicname},
            {"GET /musics", &getMusics},
            {"GET /", &get},
            {"GET /javascript", &getJS},
            {"POST /?musicname", &postMusicname}
        };

        // find method and endpoint
        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string method = request.substr(0,pos1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);
        std::string endpoint_tmp;
        if (endpoint.find("=") == -1) {
            endpoint_tmp = endpoint.substr(0,endpoint.find_last_of(" "));
        } else {
            int pos_tmp = endpoint.find("=");
            uint8_t breaker = 0;
            endpoint_tmp += endpoint.substr(0,pos_tmp);
            while(breaker < 127) { // no more than 127 iterations
                if (endpoint.find("=",pos_tmp+1) == -1) {
                    break;
                } else {
                    endpoint_tmp += endpoint.substr(
                                                endpoint.find("&",pos_tmp),
                                                endpoint.find("=",pos_tmp+1)-endpoint.find("&",pos_tmp)
                                    );
                    pos_tmp = endpoint.find("=",pos_tmp+1);
                }
                breaker++;
            }
        }
        const std::string endpoint_parameter = endpoint_tmp;

        //std::cout << endpoint_parameter << std::endl;

        // build response using appropriate function
        if (response_function.find(method+" "+endpoint_parameter) != response_function.end()) {
            builder  = response_function.at(method+" "+endpoint_parameter);
            //int pos_content_length = request.find("Content-Length: ") + 16;
            //if(pos_content_length > 16 && std::stoi(request.substr(pos_content_length,request.find("\r\n",pos_content_length)-pos_content_length)) > BUFFER_SIZE/2) {

            response = (*builder)(request);
        } else if (endpoint_parameter.length() > 1 && access(("../javascript/"+endpoint_parameter.substr(1)).c_str(), F_OK) != -1) { // requesting js file
            builder = response_function.at(method+" "+"/javascript");
            response = (*builder)(request);
        } else {
                response = getEmpty(request);
        }

		//send response (can be multiple)
        for (const auto & res : response) {
            char * response_array = new char[res.size()];
            strcpy(response_array, res.c_str());
            send(*(int *)socket,response_array, strlen(response_array),0);
            delete[] response_array;
        }
		close(*(int *)socket);

		return NULL;
	}

    std::vector<std::string> getMusicname(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

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

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> getMusics(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

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

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> get(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        // get initial page
        std::string html_file = readFileToString("../html/initial_page.html");

        // build html page response
        header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                std::to_string(html_file.size()) + "\n\n";
        body = html_file;

        response.push_back(header+body);

        return response;
    }

    std::vector<std::string> getJS(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);

        std::string js_file = readFileToString(("../javascript" + endpoint).c_str());

        //build js page response
        header = "HTTP/1.1 200 OK\nContent-Type: text/javascript\n"
                               "Content-Disposition: attachment; filename=\"playlist.js\"\n"
                                "Content-Length: " +
                                std::to_string(js_file.size()) + "\n\n";
        body = js_file;

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> getEmpty(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        body = "NULL";

        // build response
        header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                std::to_string(body.size()) + "\n\n";

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> postMusicname(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;
        std::ofstream ofile("testfile.mp3");

        // figure out how to get body from multiform request into ofile
        int boundary_pos = request.find("boundary=");
        std::string boundary = request.substr(boundary_pos+9,request.find("\r\n",boundary_pos)-boundary_pos-9);

        std::cout << boundary << std::endl;

        int first_boundary = request.find(boundary);
        body = request.substr(request.find("Content-Type: ",first_boundary));
        body = body.substr(body.find("\r\n"));

        body = body.substr(4,body.find(boundary)-4);
//        const char * file_data = body.c_str();

        ofile.write(body.c_str(),body.length());

        body = "NULL";
        header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                std::to_string(body.size()) + "\n\n";

        //if (request.find(boundary,boundary_pos+10) != -1) std::cout << "there is a second boundary" << std::endl;

        response.push_back(header+body);
        return response;
    }

    std::string getRequestAsString(const void * socket, const int buffer_size) {
        
		std::vector<char> buffer(buffer_size);
        static std::string request;
        std::string cur_request; // request as string
        static int cumulative_bytes = 0;
        int bytes_received;
        static int read_attempts = 0;

        // cumulate data

		bytes_received = read(*(int *)socket, &buffer[0], buffer_size); // receive request data from client and store into buffer

        // test if content was read
        if (bytes_received < 0) {
			exitWithError("Failed to read bytes from client connection");
		}

        cur_request.assign(&buffer[0],bytes_received);
        request.append(cur_request);
        cumulative_bytes += bytes_received;


        if(bytes_received == buffer_size && read_attempts < 100) {
            read_attempts++;
            return getRequestAsString(socket, buffer_size);
        }

        std::string return_string = request;

        // clearing static variables
        request.clear();
        read_attempts = 0;
        cumulative_bytes = 0;

        return return_string;

    }
}