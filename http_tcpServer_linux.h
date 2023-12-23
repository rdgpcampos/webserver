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
#include <unistd.h>
#include "../../../../Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/sys/fcntl.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <semaphore>
#include <queue>

// forward declaration of handler to remove dependency
namespace handler {
	void * handle_client(void * client);
}

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
		//long m_incoming_message;
		struct sockaddr_in m_socket_address;
		unsigned int m_socket_address_len;
		std::string m_server_message;

		int startServer();
		void closeServer();
		void acceptConnection(int &new_socket);
		std::string buildResponse();
		void sendResponse(const char * message, const int length);
	};

	std::string getRequestAsString(const void * socket, const int buffer_size);
    int sendWithRetry(int socket, const char * message, int message_size);
	int readWithRetry(const void * socket, char * buffer, int buffer_size);
/*
	class MessageTransfer {
	public:
		MessageTransfer(const int socket, const int message_size);
		~MessageTransfer();
		void transferMessage();

	private:
		std::counting_semaphore<util::BUFFER_SIZE> number_of_queueing_positions{0};
		std::counting_semaphore<util::BUFFER_SIZE> number_of_empty_positions{util::BUFFER_SIZE};
		std::mutex buffer_manipulation;
		std::queue<char> bounded_buffer;
		const int socket;
		const int message_size;

		void producer();
		void consumer();
	};
*/

}

#endif


