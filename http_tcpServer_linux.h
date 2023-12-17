#ifndef INCLUDED_HTTP_TCPSERVER_LINUX
#define INCLUDED_HTTP_TCPSERVER_LINUX

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
#include "util.h"
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>

#include "client_handler.h"


namespace http {
	class TcpServer {
	public:
		TcpServer(std::string ip_address, int port);
		~TcpServer();
		void startListen();

	private:
		std::string m_ip_address;
		int m_port;
		int m_socket;
		int m_new_socket;
		long m_incoming_message;
		struct sockaddr_in m_socket_address;
		unsigned int m_socket_address_len;
		std::string m_server_message;

		int startServer();
		void closeServer();
		void acceptConnection(int &new_socket);
		std::string buildResponse();
		void sendResponse(const char * message, const int length);
	};
}

#endif


