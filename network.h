#ifndef _NETWORK_
#define _NETWORK_


#include <Windows.h>
#include <winsock.h>
#include <deque>
#include <vector>

#include "client.h"

#define PORT 9000

class Network
{
public:
	static Network *getInstance()
	{
		if (instance == NULL)
			instance = new Network();
		return instance;
	};

	void	reset();
	void	run();
	bool	sendpacket(SocketBuffer buffer);
	void	removeclient(Client *client);

private:
	static Network *	instance;

	Network();
	~Network()
	{
		WSACleanup();
	}


	SOCKET serversock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	std::vector<Client*> clientlist;
	std::vector<Client*> removelist;
};

#endif
