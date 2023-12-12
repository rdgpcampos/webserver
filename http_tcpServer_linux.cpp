#include <http_tcpServer_linux.h>

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <map>
#include <regex>
#include <fstream>


namespace {
	const int BUFFER_SIZE = 30720;
	typedef unsigned uchar;
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
			exit(1);
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

	std::string readFileAsBinary(const char * filename) {

		std::ifstream infile(filename, std::ios::binary);
		if (!infile) {
			fputs("File could not be opened", stderr);
			exit(1);
		}

		std::string decoded_string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

		return decoded_string;
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
							//"Content-Type: multipart/form-data; boundary: myboundary\r\n"
							"Content-Type: application/json\r\n"
							"Accept: */*\r\n"
							"Connection: close\r\n"
							"\r\n";
							//"--myboundary\r\n"
							//"Content-Type: audio/x-wav\r\n"
							//"Content-Disposition: attachment; name=\"audiofile\"; filename=\"myaudiofile.wav\"\r\n"
							//"Content-Transfer-Encoding: base64\r\n";

			send(socket, header, strlen(header),0);

			char * response_array = new char[response.size()+13];
			response = "{\"data\": \"" + response + "\"}";
			strcpy(response_array, response.c_str());

			send(socket,response_array, strlen(response_array),0);

			delete[] response_array;

			//char * footer = "\r\n"
			//				"--myboundary--\r\n";
			//send(socket,footer,strlen(footer),0);

			return "";
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
