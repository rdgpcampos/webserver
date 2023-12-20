#include <http_tcpServer_linux.h>

int main() {
	using namespace http;

	TcpServer server = TcpServer("127.0.0.1",util::getPort());
	server.startListen();

	return 0;
}
