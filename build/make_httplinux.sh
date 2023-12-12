#!/bin/sh

rm HttpLinux
gcc -I.. -o HttpLinux ../server_linux.cpp ../http_tcpServer_linux.cpp -lstdc++
