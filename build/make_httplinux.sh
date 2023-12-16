#!/bin/sh

rm HttpLinux
c++ -std=c++11 -I.. -o HttpLinux ../util.cpp ../server_linux.cpp ../client_handler.cpp ../http_tcpServer_linux.cpp -lstdc++ -lcurl
