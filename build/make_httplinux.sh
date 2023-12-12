#!/bin/sh

rm HttpLinux
c++ -std=c++11 -I.. -o HttpLinux ../server_linux.cpp ../http_tcpServer_linux.cpp -lstdc++ -lcurl
