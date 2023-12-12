#include <http_tcpServer_linux.h>

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <map>
#include <regex>

namespace {
	const int BUFFER_SIZE = 30720;

	void log(const std::string &message) {
		std::cout << message << std::endl;
	}

	void exitWithError(const std::string &error_message) {
		log("ERROR: " + error_message);
		exit(1);
	}

	std::string readFileToString(const char * file_name) {
		FILE *fp;
		long l_size;
		size_t result;
		char * buffer;
		
		fp = fopen(file_name,"rb");
		if (fp == NULL) {
			fputs("Could not load file",stderr);
			exit(1);
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

		return output;
	}

	void handle_client(int socket) {
		char buffer[BUFFER_SIZE] = {0};

		// receive request data from client and store into buffer
		int bytes_received = read(socket, buffer, BUFFER_SIZE);

		if (bytes_received < 0) {
			exitWithError("Failed to read bytes from client connection");
		}

		std::string request(buffer);
		std::smatch m;
		std::cout << request << std::endl;

		// match GET request
		if(std::regex_match(request, std::regex("^GET /([^ ]*) HTTP/1(.|\\n|\\r)*$"))) {
			
		}

		return;
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

			handle_client(m_new_socket);

			std::ostringstream ss;
			ss << "--------Received request from client--------\n\n";
			log(ss.str());

			// create a new thread to handle client request
			//pthread_t thread_id;
			//pthread_create(&thread_id, NULL, handle_client, (void *)m_new_socket);
			//pthread_detach(thread_id);
			//std::cout<< "Passing socket to handler" << std::endl;

			sendResponse();


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
		std::string script = readFileToString("../javascript/playBytes.js");

		std::string html_file = "<!DOCTYPE html><html lang=\"en\"><body><script>"+script+"</script><h1> HOME </h1><p>Hello world! :)</p></body></html>";
		std::ostringstream ss;
		ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html_file.size() << "\n\n" << html_file;

		return ss.str();
	}

	void TcpServer::sendResponse() {
		long bytes_sent;

		bytes_sent = write(m_new_socket, m_server_message.c_str(), m_server_message.size());

		if (bytes_sent == m_server_message.size()) {
			log("------- Server response sent to client -------\n\n");
		} else {
			log("Error sending response to client");
		}
	}
}
