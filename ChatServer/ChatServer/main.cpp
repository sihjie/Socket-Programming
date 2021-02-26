// ChatServer
# include <iostream>
#include <ws2tcpip.h>
#include <sstream>
#include <map>

#pragma comment (lib, "ws2_32.lib")	

using namespace std;

void main() {
	map<SOCKET, string> mapName;

	// Initialize Winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOK = WSAStartup(ver, &wsData);
	if (wsOK != 0) {
		cerr << "Can't Initialize winsock!" << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket." << endl;
		return;
	}

	// Bind the the IP address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;		//Could also use inet_pton

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening
	listen(listening, SOMAXCONN);


	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true) {
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];
			if (sock == listening) {
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				char name_buf[4096];
				ZeroMemory(name_buf, 4096);
				string welcomeMsg = "Welcome to the Chat Server!\r\nPlease enter your name : ";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
				int nameIn = recv(client, name_buf, 4096, 0);
				cout << "Successfully connected to " << string(name_buf, 0, nameIn) << endl;

				// Bind the client and its name
				mapName[client] = string(name_buf, 0, nameIn);
			}
			else {	// Accept a new message
				char buf[4096];
				ZeroMemory(buf, 4096);
				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					// Send message to other clients, and definately NOT the listening socket

					for (int i = 0; i < master.fd_count; i++) {
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock) {
							ostringstream sout;

							sout << mapName[sock] << ": " << buf << "\r\n";

							string strOut = sout.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
}





