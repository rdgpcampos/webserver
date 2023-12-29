#include <http_tcpServer_linux.h>

namespace http {
	TcpServer::TcpServer(std::string ip_address, int port) :
	m_ip_address(ip_address), m_port(port), m_socket(),
	m_new_socket(), m_socket_address(),
	m_socket_address_len(sizeof(m_socket_address)), m_server_message(buildResponse()) 
	{
		m_socket_address.sin_family = AF_INET;
		m_socket_address.sin_port = htons(m_port);
		m_socket_address.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

		if(startServer() != 0) {
			std::ostringstream ss;
			ss << "Failed to start server with PORT: " << ntohs(m_socket_address.sin_port); 
			util::log(ss.str());
		}
	}
	TcpServer::~TcpServer() {
		closeServer();
	}

	int TcpServer::startServer() {
		m_socket = socket(AF_INET,SOCK_STREAM,0);

		if (m_socket < 0) {
			util::exitWithError("Cannot create socket");
			return 1;
		}

		if(bind(m_socket, (sockaddr *)&m_socket_address, m_socket_address_len)) {
			util::exitWithError("Cannot connect socket to address");
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
			util::exitWithError("Socket listen failed");
		}

		std::ostringstream ss;
		ss << "\n*** Listening on ADDRESS: "
		   << inet_ntoa(m_socket_address.sin_addr)
		   << " PORT: " << ntohs(m_socket_address.sin_port)
		   << " ***\n\n ";
		util::log(ss.str());

		while(true) {
			util::log("============Waiting for a new connection============\n\n\n");
			acceptConnection(m_new_socket);
			util::log("--------Received request from client--------\n\n");

			// create a new thread to handle client request
			pthread_t thread_id;
			int *socket = (int *)malloc(sizeof(*socket));
			*socket = m_new_socket;

			int flags;
			if (-1 == (flags = fcntl(m_new_socket, F_GETFL, 0))) flags = 0;
			// setting non-blocking socket
			fcntl(m_new_socket, F_SETFL, flags | O_NONBLOCK);

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
			util::exitWithError(ss.str()); 
		}
	}

	std::string TcpServer::buildResponse() {
		std::string html_file = util::readFileToString("../html/initial_page.html");
		std::ostringstream ss;
		ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html_file.size() << "\n\n" << html_file;

		return ss.str();
	}

	void TcpServer::sendResponse(const char * message, const int length) {
		long bytes_sent = 0;
		if (length > 0) 
			bytes_sent = send(m_new_socket, message, length, 0);

		if (bytes_sent == (long)std::string(message).size()) {
			util::log("------- Server response sent to client -------\n\n");
		} else {
			util::log("Error sending response to client");
		}
	}

	// read from socket multiple times until request is consumed entirely
	// this assumes that request fits in RAM, which is not the case for large files (videos for instance)
    std::string getRequestAsString(const void * socket, const int buffer_size) {
		std::vector<char> buffer(buffer_size);
        std::string cur_request; // request as string
        int bytes_received;
		static int body_length = 0;
        static std::string request = "";
        static int cumulative_bytes = 0;
        static int read_attempts = 0;
        static int timeout_attempt = 0;

		static int pos1 = 0;
		static int pos2 = 0;

        // read from socket
		bytes_received = readWithRetry(socket, &buffer[0], buffer_size); // receive request data from client and store into buffer

        // test if content was read
        if (bytes_received < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            timeout_attempt++;
            return timeout_attempt>5? "500" : getRequestAsString(socket, buffer_size);
		}

        // get read content
        cur_request.assign(&buffer[0],bytes_received);
		if (cumulative_bytes == 0) {
			pos1 = cur_request.find("Content-Length: ");
			if (pos1 > 0) pos2 = cur_request.find("\n",pos1);
			if (pos1 > 0) body_length = std::stoi(cur_request.substr(pos1+16,pos2-pos1-16));
		}
        // cumulate content
        request.append(cur_request);

        cumulative_bytes += bytes_received;



        // repeat until request is entirely consumed from socket or max limit is reached
		if(body_length > 0 && cumulative_bytes < body_length && read_attempts < 100) {
            read_attempts++;
            return getRequestAsString(socket, buffer_size);
        }

        std::string return_string = request;

        if (read_attempts == 100) {
            return "413";
        }

        // clearing static variables
        request.clear();
        read_attempts = 0;
        cumulative_bytes = 0;
		body_length = 0;

        return return_string;

    }

	int readWithRetry(const void * socket, char * buffer, int buffer_size) {
		int bytes_received = 0;
		int read_attempts = 0;

		do {
			bytes_received = read(*(int *)socket, buffer, buffer_size);

			if (bytes_received < 0) {
				if (read_attempts++ > 5) break;
				std::this_thread::sleep_for(std::chrono::milliseconds(50)); // try again after 20 milliseconds
				bytes_received = 0;
			}
		} while (bytes_received == 0);

		return bytes_received;
	}

    int sendWithRetry(int socket, const char * message, int message_size) {
        int total_bytes_sent = 0;
        int bytes_sent_now = 0;
        int send_attempts = 0;

        do {
            bytes_sent_now = send(socket, &message[total_bytes_sent], message_size-total_bytes_sent, 0);

            if (bytes_sent_now > 0) {
				total_bytes_sent += bytes_sent_now;
				send_attempts = 0;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(20)); // try again after 20 milliseconds
                if (send_attempts++ > 10) return -1; // fail too many times
            }
        } while (total_bytes_sent < message_size);

        return total_bytes_sent;
    }

/*
	MessageTransfer::MessageTransfer(const int socket, const int message_size) : socket(socket), message_size(message_size) {};
	MessageTransfer::~MessageTransfer() {};

	// Dijkstra's bounded buffer
	void MessageTransfer::transferMessage() {
		std::thread t1([this](){this->producer();});
		std::thread t2([this](){this->consumer();});
		t1.join();
		t2.join();
	}

	void MessageTransfer::producer() {
		std::vector<char> in_buffer(100);
		int bytes_in_buffer = 0;
		int bytes_received = 0;

		while(bytes_received < message_size) {
			if (bytes_in_buffer == 0) {
				in_buffer.clear();
				bytes_in_buffer = readWithRetry((void *)&socket, &in_buffer[0], 100);
				bytes_received += bytes_in_buffer;
			}
			number_of_empty_positions.acquire();
			{
				std::lock_guard<std::mutex> g(buffer_manipulation);
				bounded_buffer.push(in_buffer[bytes_in_buffer]);
				bytes_in_buffer--;
			}
			number_of_queueing_positions.release();
		}
	}

	void MessageTransfer::consumer() {
		std::vector<char> out_buffer(100);
		int bytes_in_buffer = 0;
		int bytes_sent = 0;
		//int bytes_sent_now = 0;

		while(bytes_sent < message_size) {
			number_of_queueing_positions.acquire();
			{
				std::lock_guard<std::mutex> g(buffer_manipulation);
				out_buffer[bytes_in_buffer] = bounded_buffer.front();
				bytes_in_buffer++;
				bounded_buffer.pop();
			}
			number_of_empty_positions.release();
			if (bytes_in_buffer == 100) {
				//bytes_sent_now = sendWithRetry(socket, &out_buffer[0], bytes_in_buffer); 
				std::cout << &out_buffer << std::endl;
				out_buffer.clear();
				bytes_sent -= 100;
				bytes_sent += 100;  
			}
		}
	}
*/
}
