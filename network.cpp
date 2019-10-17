#include "Network.h"

Network * Network::instance = NULL;

Network::Network()
{
	serversock = INVALID_SOCKET;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		printf("error\r\n");

	serversock = socket(AF_INET, SOCK_STREAM, 0);
	if (serversock == INVALID_SOCKET)
	{
		printf("ERROR opening socket");
		return;
	}

	int sockopt = 1;
	if (setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, (char*)&sockopt, sizeof(sockopt)) == -1)
	{
		printf("error socket option : REUSEADDR");
		return;
	}

	ZeroMemory(&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	int status = bind(serversock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in));
	if (status == SOCKET_ERROR)
		printf("Bind Error\n");

	listen(serversock, 5);
	printf("Network initialized !!!\n");
}

void Network::run()
{
	for (int i = 0; i < removelist.size(); i++)
	{
		std::vector<Client*>::iterator it = clientlist.begin();
		for (; it != clientlist.end(); it++)
		{
			if (removelist[i] == *it)
			{
				delete removelist[i];
				clientlist.erase(it);
				break;
			}
		}
	}

	if(!removelist.empty())
		removelist.clear();

	//////////////////////////////////////////////////////////////////////////

	fd_set readfds, allfds;
	FD_ZERO(&readfds);
	FD_SET(serversock, &readfds);
	allfds = readfds;

	struct    timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;

	select(serversock + 1, &allfds, NULL, NULL, &tv);

	if (FD_ISSET(serversock, &allfds)) {

		ZeroMemory(&cli_addr, sizeof(struct sockaddr_in));
		int sockLen = sizeof(struct sockaddr_in);
		SOCKET clisock = accept(serversock, (struct sockaddr*)&cli_addr, &sockLen);
		if (clisock == INVALID_SOCKET)
		{
			printf("Accept Error");
			closesocket(serversock);
			return;
		}

		// client socket --> nonblock
		unsigned long arg = 1;
		if (ioctlsocket(clisock, FIONBIO, &arg) != 0) return;

		printf("New client comming !!\n");

		Client *client = new Client();
		client->clisock = clisock;
		clientlist.push_back(client);
	}

	// 모두 업데이트
	std::vector<Client*>::iterator it = clientlist.begin();
	for (; it != clientlist.end(); it++)
	{
		(*it)->run();
	}
}

bool	Network::sendpacket(SocketBuffer buffer)
{
	std::vector<Client*>::iterator it = clientlist.begin();
	for (; it != clientlist.end(); it++)
	{
		(*it)->sendpacket(buffer);
	}

	return true;
}

void	Network::removeclient(Client *client)
{
	removelist.push_back(client);
}

void	Network::reset()
{
	if (serversock != INVALID_SOCKET)
		closesocket(serversock);
}
