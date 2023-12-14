#include <http_tcpServer_linux.h>

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <map>
#include <regex>
#include <fstream>
#include <filesystem>
#include "util.h"

namespace fs = std::__fs::filesystem;


namespace {
	const int BUFFER_SIZE = 30720;
	typedef unsigned uchar;

	std::string handle_client(int socket) {
		char buffer[BUFFER_SIZE] = {0};

		// receive request data from client and store into buffer
		int bytes_received = read(socket, buffer, BUFFER_SIZE);

		if (bytes_received < 0) {
			exitWithError("Failed to read bytes from client connection");
		}

		// send music
		if(std::regex_match(buffer, std::regex("^GET /.musicname([^ ]*) HTTP/1(.|\\n|\\r)*$"))) {
			std::ostringstream ss;
			const std::string request(buffer);
			int pos1 = request.find("musicname");
			int pos2 = request.find("HTTP");
			std::string filename_string = "../playlist/" + request.substr(pos1+10,pos2-pos1-11);
			
			const char * filename = &filename_string[0];

			std::string response = encodeFile(filename);


			char * header = "HTTP/1.1 200 OK /audiofile.mp3\r\n"
							"Content-Type: application/json\r\n"
							"Accept: */*\r\n"
							"Connection: close\r\n"
							"\r\n";

			send(socket, header, strlen(header),0);

			char * response_array = new char[response.size()+13];
			response = "{\"data\": \"" + response + "\"}";
			strcpy(response_array, response.c_str());

			send(socket,response_array, strlen(response_array),0);

			delete[] response_array;

			return "";
		}

		// send playlist
		if(std::regex_match(buffer, std::regex("^GET /musics HTTP/1(.|\\n|\\r)*$"))) {
			std::ostringstream ss;
			const std::string request(buffer);

			// prepare response containing playlist
			std::string response = "[";
			std::string playlist_path = "../playlist";
			for (const auto & music : fs::directory_iterator(playlist_path)) {
				response = response + 
							"{\"name\":\"" +
							music.path().string().substr(playlist_path.length()+1) +
							"\"},";
			}
			response = response.substr(0,response.length()-1) + "]";

			char * header = "HTTP/1.1 200 OK \r\n"
				"Content-Type: application/json\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"Content-Length: ";

			ss << header << std::to_string(response.length()) << "\r\n\r\n" << response;

			return ss.str();
		}

		// match GET request
		//std::cout << buffer << std::endl; // see request string shape to adjust regex
		if(std::regex_match(buffer, std::regex("^GET /([^ ]*) HTTP/1(.|\\n|\\r)*$"))) {
			std::string html_file = readFileToString("../html/initial_page.html");
			std::ostringstream ss;
			ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html_file.size() << "\n\n" << html_file;

			return ss.str();
		}

		return "";
	}
}

namespace http {
	TcpServer::TcpServer(std::string ip_address, int port) :
	m_ip_address(ip_address), m_port(port), m_socket(),
	m_new_socket(), m_incoming_message(), m_socket_address(),
	m_socket_address_len(sizeof(m_socket_address)), m_server_message(buildResponse()) 
	{
		m_socket_address.sin_family = AF_INET;
		m_socket_address.sin_port = htons(m_port);
		m_socket_address.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

		if(startServer() != 0) {
			std::ostringstream ss;
			ss << "Failed to start server with PORT: " << ntohs(m_socket_address.sin_port); 
			log(ss.str());
		}
	}
	TcpServer::~TcpServer() {
		closeServer();
	}

	int TcpServer::startServer() {
		m_socket = socket(AF_INET,SOCK_STREAM,0);
		if (m_socket < 0) {
			exitWithError("Cannot create socket");
			return 1;
		}

		if(bind(m_socket, (sockaddr *)&m_socket_address, m_socket_address_len)) {
			exitWithError("Cannot connect socket to address");
			return 1;
		}

		return 0;
	}

	void TcpServer::closeServer() {
		close(m_socket);
		close(m_new_socket);
		exit(0);
	}

	void TcpServer::startListen() {
		if (listen(m_socket, 20) < 0) {
			exitWithError("Socket listen failed");
		}

		std::ostringstream ss;
		ss << "\n*** Listening on ADDRESS: "
		   << inet_ntoa(m_socket_address.sin_addr)
		   << " PORT: " << ntohs(m_socket_address.sin_port)
		   << " ***\n\n ";
		log(ss.str());

		while(true) {
			log("============Waiting for a new connection============\n\n\n");
			acceptConnection(m_new_socket);

			char * response = (char *)malloc(BUFFER_SIZE * sizeof(char));
			//handle_client(m_new_socket);
			strcpy(response, handle_client(m_new_socket).c_str());
			sendResponse(response);
			free(response);

			std::ostringstream ss;
			ss << "--------Received request from client--------\n\n";
			log(ss.str());

			// create a new thread to handle client request
			//pthread_t thread_id;
			//pthread_create(&thread_id, NULL, handle_client, (void *)m_new_socket);
			//pthread_detach(thread_id);
			//std::cout<< "Passing socket to handler" << std::endl;

			close(m_new_socket);
		}
	}

	void TcpServer::acceptConnection(int &new_socket) {
		new_socket = accept(m_socket, (sockaddr *)&m_socket_address,&m_socket_address_len);
		if (new_socket < 0) {
			std::ostringstream ss;
			ss << "Server failed to accept incoming request from ADDRESS: "
			   << inet_ntoa(m_socket_address.sin_addr)
			   << "; PORT: "
			   << ntohs(m_socket_address.sin_port);
			exitWithError(ss.str()); 
		}
	}

	std::string TcpServer::buildResponse() {
		std::string html_file = readFileToString("../html/initial_page.html");
		std::ostringstream ss;
		ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html_file.size() << "\n\n" << html_file;

		return ss.str();
	}

	void TcpServer::sendResponse(char * message) {
		long bytes_sent;
		if (strlen(message) > 0) 
			bytes_sent = write(m_new_socket, std::string(message).c_str(), std::string(message).size());

		if (bytes_sent == std::string(message).size()) {
			log("------- Server response sent to client -------\n\n");
		} else {
			log("Error sending response to client");
		}
	}
}
