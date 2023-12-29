#include <client_handler.h>

/*

What's left to do:
    1. Set http responses for fail cases (DONE)
    2. Identify common string parsing operations and put them into separate functions (ANOTHER TIME)
    3. Prettify UI
    4. Put playlist path and port as app settings (DONE)
    5. util dependency is hidden, so add namespace (DONE)
    6. Limit download type to audio files - mp3, ogg/oga/mogg, wav (DONE)
    7. Use a producer/consumer pattern to allow bigger files to be uploaded (WILL MOVE TO DIFFERENT PROJECT)
    8. Now that socket is non-blocking, need to create specialized functions to read and write to it (DONE)
    9. Remove socket communication responsibility from handler and give it to http (DONE)

*/

namespace handler {
	void * handle_client(void * socket) {
        std::vector<std::string> response;
        buildResponse builder;
        std::string request = http::getRequestAsString(socket, util::BUFFER_SIZE);

        // error handling
        if (request == "500") {
            response.push_back("HTTP/1.1 500 Internal Server Error\r\n\r\nCould not read data from request");
            char * response_array = new char[response[0].size()];
            strcpy(response_array, response[0].c_str());
            send(*(int *)socket,response_array, strlen(response_array),0);
            delete[] response_array;
            close(*(int *)socket);
            return NULL;

        } else if (request == "413") {
            response.push_back("HTTP/1.1 413 Payload Too Large\r\nFile exceeds size limit");
            char * response_array = new char[response[0].size()];
            strcpy(response_array, response[0].c_str());
            send(*(int *)socket,response_array, strlen(response_array),0);
            delete[] response_array;
            close(*(int *)socket);
            return NULL;
        }

        // valid endpoints and their respective handler functions
        const std::unordered_map<std::string, buildResponse> response_function = {
            {"GET /?musicname",&getMusicname},
            {"GET /musics", &getMusics},
            {"GET /favicon.ico", &getFavicon},
            {"GET /", &get},
            {"GET /javascript", &getJS},
            {"GET /images",&getImage},
            {"POST /?musicname", &postMusicname},
            {"DELETE /?musicname", &deleteMusicname}
        };

        // find method and endpoint
        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string method = request.substr(0,pos1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);
        std::string endpoint_tmp;
        if (endpoint.find("=") == (size_t)-1) {
            endpoint_tmp = endpoint.substr(0,endpoint.find_last_of(" "));
        } else {
            int pos_tmp = endpoint.find("=");
            uint8_t breaker = 0;
            endpoint_tmp += endpoint.substr(0,pos_tmp);
            while(breaker < 127) { // no more than 127 iterations
                if (endpoint.find("=",pos_tmp+1) == (size_t)-1) {
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

        // build response using appropriate function
        if (response_function.find(method+" "+endpoint_parameter) != response_function.end()) {
            builder  = response_function.at(method+" "+endpoint_parameter);
            response = (*builder)(request);

        } else if (endpoint_parameter.length() > 1 && access(("../javascript/"+endpoint_parameter.substr(1)).c_str(), F_OK) != -1) { // requesting js file
            builder = response_function.at(method+" "+"/javascript");
            response = (*builder)(request);
        } else if (endpoint_parameter.length() > 1 && access(("../images/"+endpoint_parameter.substr(1)).c_str(), F_OK) != -1) { // requesting image file
            builder = response_function.at(method+" "+"/images");
            response = (*builder)(request);
        } else {
            response = getEmpty(request);
        }

		//send response (can be multiple)
        for (const auto & res : response) {
            char * response_array = new char[res.size()];
            strcpy(response_array, res.c_str());
            http::sendWithRetry(*(int *)socket,response_array, res.size());
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

        std::string filename_string = util::getPlaylistDirectory()+ "/" + request.substr(pos1+10,pos2-pos1-11);

        char * filename = &filename_string[0];

        // build body
        body = util::encodeFile(filename); // convert to base64

        // build response
        if (body == "") {
            header = "HTTP/1.1 404 File could not be opened\r\n";
        } else {
            header = "HTTP/1.1 200 OK /audiofile.mp3\r\n"
                            "Content-Type: application/json\r\n"
                            "Accept: */*\r\n"
                            "Connection: close\r\n"
                            "\r\n";

            body = "{\"data\": \"" + body + "\"}";
        }

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
        std::string playlist_path = util::getPlaylistDirectory();
        for (const auto & music : fs::directory_iterator(playlist_path)) {
            if(music.path().string().substr(playlist_path.length()+1,1) == ".") continue;
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

    std::vector<std::string> getFavicon(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        header = "HTTP/1.1 200 OK\r\n";

        response.push_back(header+body);

        return response;
    }

    std::vector<std::string> get(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        // get initial page
        std::string html_file = util::readFileToString("../html/initial_page.html");
        body = html_file;

        // build html page response
        if (body == "") {
            header = "HTTP/1.1 404 Page could not be loaded";
        } else {
            header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + 
                                    std::to_string(html_file.size()) + "\r\n\r\n";
        }

        response.push_back(header+body);

        return response;
    }

    std::vector<std::string> getJS(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        // get request endpoint to find corresponding javascript file
        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);

        // read javascript file
        std::string js_file = util::readFileToString(("../javascript" + endpoint).c_str());

        body = js_file;

        // build response
        if (body == "") {
            header = "HTTP/1.1 404 File could not be opened\r\n\r\n";
        } else {
            //build js page response
            header = "HTTP/1.1 200 OK\nContent-Type: text/javascript\n"
                                "Content-Disposition: attachment; filename=\"playlist.js\"\n"
                                    "Content-Length: " +
                                    std::to_string(body.size()) + "\r\n\r\n"; 
        }

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> getEmpty(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        // build response
        header = "HTTP/1.1 501 Not Implemented\r\n\r\n";
        body = "No way to respond to this request\r\n";

        response.push_back(header+body);
        return response;
    }

    std::vector<std::string> postMusicname(std::string request) {
        std::string header;
		std::string body;
        std::string file_body;
        std::vector<std::string> response;


        // set filename as musicname (if name already exists, add a number between brackets to it)
        int pos1 = request.find("musicname=");
        int pos2 = request.find(" ",pos1);
        const std::string musicname = util::getPlaylistDirectory() + "/" + request.substr(pos1+10,pos2-pos1-10);

        std::string extension = musicname.substr(musicname.find_last_of(".")+1);
        if (!(extension == "mp3" || extension == "wav" || extension == "oga" || extension == "ogg" || extension == "mogg")) {
            header = "HTTP/1.1 415 Unsupported Media Type\r\n\r\n";
            body = "Only support mp3, wav, oga, ogg, mogg file types";
            response.push_back(header+body);
            return response;
        }

        std::ofstream ofile(util::setFileName(musicname));

        if (ofile.is_open()) { // check if the file was opened successfully
            // find boundary in request
            int boundary_pos = request.find("boundary=");
            std::string boundary = request.substr(boundary_pos+9,request.find("\r\n",boundary_pos)-boundary_pos-9);
            int first_boundary = request.find(boundary);

            // extract file body from request
            file_body = request.substr(request.find("Content-Type: ",first_boundary));
            file_body = file_body.substr(file_body.find("\r\n"));
            file_body = file_body.substr(4,file_body.find(boundary)-8);
            ofile << file_body;
            ofile.close(); // close the file when done

            // build response
            body = "";
            header = "HTTP/1.1 200 OK\r\n\r\n";
        } else {
            header = "HTTP/1.1 400 Bad Request\r\n\r\n";
            body = "Failed to open file, try again later\n";
        }

        if ( ofile.bad() ) {
            header = "HTTP/1.1 400 Bad Request\n\n";
            body = "Failed to upload file, try again later\n";
        }

        response.push_back(header+body);

        return response;

    }

    std::vector<std::string> getImage(std::string request) {
        std::string header;
        std::vector<std::string> response;

         // get request endpoint to find corresponding javascript file
        int pos1 = request.find(" ");
        int pos2 = request.find(" ",pos1+1);
        const std::string endpoint = request.substr(pos1+1,pos2-pos1-1);

        // read image file
        std::string body = util::readFileToString(("../images" + endpoint).c_str());

        // build response
        if (body.size() == 0) {
            header = "HTTP/1.1 404 File could not be opened\r\n\r\n";
        } else {
            //build js page response
            header = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\n"
                                "Content-Disposition: attachment; filename=\""+endpoint+"\"\n"
                                    "Content-Length: " +
                                    std::to_string(body.size()) + "\r\n\r\n"; 
        }

        response.push_back(header);
        response.push_back(body);
        return response;
    }

    // delete audio file
    std::vector<std::string> deleteMusicname(std::string request) {
        std::string header;
		std::string body;
        std::vector<std::string> response;

        // get music name
        int pos1 = request.find("musicname=");
        int pos2 = request.find(" ",pos1);
        const std::string musicname = util::getPlaylistDirectory() + "/" + request.substr(pos1+10,pos2-pos1-10);

        if (fs::remove(musicname)) {
            header = "HTTP/1.1 200 OK\r\n\r\n";
        } else {
            header = "HTTP/1.1 404 File not found\n";
        }

        body = "";

        response.push_back(header+body);
        return response;
    }
}