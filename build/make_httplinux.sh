#!/bin/sh

rm HttpLinux
c++ -std=c++20 -I.. -Wall -Wextra -o HttpLinux ../util.cpp ../server_linux.cpp ../client_handler.cpp ../http_tcpServer_linux.cpp -lstdc++ -lcurl
