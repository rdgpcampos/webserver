#include <http_tcpServer_linux.h>

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
			log("--------Received request from client--------\n\n");

			// create a new thread to handle client request
			pthread_t thread_id;
			int *socket = (int *)malloc(sizeof(*socket));
			*socket = m_new_socket;
			pthread_create(&thread_id, NULL, &handler::handle_client, (void *)socket);
			pthread_detach(thread_id);
			std::cout<< "Passing socket to handler" << std::endl;
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

	void TcpServer::sendResponse(const char * message, const int length) {
		long bytes_sent;
		if (length > 0) 
			bytes_sent = send(m_new_socket, message, length, 0);

		if (bytes_sent == std::string(message).size()) {
			log("------- Server response sent to client -------\n\n");
		} else {
			log("Error sending response to client");
		}
	}
}
