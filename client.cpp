#include "predef.h"
#include "client.h"
#include "network.h"

#include <random>
#include <string>

#pragma  comment (lib,"winmm.lib")

#define MINDELAY	300
#define MAXDELAY	1000

Client::Client()
{
	process = PRO_START;
	currentcount = timeGetTime();
	randcount = 0;
}

Client::~Client()
{

}


void Client::init()
{
	clisock = socket(AF_INET, SOCK_STREAM, 0);
	if (clisock == INVALID_SOCKET)
	{
		printf("ERROR opening socket");
		return;
	}

	int sockopt = 1;
	if (setsockopt(clisock, SOL_SOCKET, SO_REUSEADDR, (char*)&sockopt, sizeof(sockopt)) == -1)
	{
		printf("error socket option : REUSEADDR");
		return;
	}

	unsigned long arg = 1;
	if (ioctlsocket(clisock, FIONBIO, &arg) != 0) return;

	struct sockaddr_in clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientaddr.sin_port = htons(PORT);

	int client_len = sizeof(clientaddr);

	connect(clisock, (struct sockaddr *)&clientaddr, client_len);

	process = PRO_SWITCH;

	currentcount = timeGetTime();
	randcount = currentcount + (MINDELAY + (rand() % MAXDELAY));

	nextprocess = PRO_CREATE_ACCOUNT;

	printf("Client connected!!\n");
}

void	Client::uninit()
{
	clearbuffer();
	// disconnected
	closesocket(clisock);
	clisock = INVALID_SOCKET;
	printf("Client disconnected!!\n");
}

void	Client::processing()
{
	if (process == PRO_START)
	{
		init();
	}
	else if (process == PRO_CREATE_ACCOUNT)
	{
		ReqCreateAccount create;

		name = getString(15);
		pass = getString(8);

		create.set_name(name);
		create.set_passwd(pass);

		int pbsize = create.ByteSize();
		PacketHeader header;
		header.signature = CLIENT_SIG;
		header.packetsize = pbsize + sizeof(PacketHeader);		// packet size는 header + data 포함한 전체 길이
		header.packetserial = 0;
		header.packetID = GameMsgID::MSG_REQ_CREATE_ACCOUNT;

		SocketBuffer buf;

		memcpy(buf.buffer, &header, sizeof(PacketHeader));
		if (!create.SerializePartialToArray(buf.buffer + sizeof(PacketHeader), pbsize))
			return;

		buf.packetsize = header.packetsize;

		sendpacket(buf);
		process = PRO_WAIT;
	}
	else if (process == PRO_LOGIN)
	{
		ReqLogin login;

		login.set_name(name);
		login.set_passwd(pass);

		int pbsize = login.ByteSize();
		PacketHeader header;
		header.signature = CLIENT_SIG;
		header.packetsize = pbsize + sizeof(PacketHeader);
		header.packetserial = 0;
		header.packetID = GameMsgID::MSG_REQ_LOGIN;

		SocketBuffer buf;

		memcpy(buf.buffer, &header, sizeof(PacketHeader));
		if (!login.SerializePartialToArray(buf.buffer + sizeof(PacketHeader), pbsize))
			return;

		buf.packetsize = header.packetsize;

		sendpacket(buf);
		process = PRO_WAIT;
	}
	else if (process == PRO_QUIT)
	{
		uninit();

		process = PRO_SWITCH;
		currentcount = timeGetTime();
		randcount = currentcount + (MINDELAY + (rand() % MAXDELAY));
		nextprocess = PRO_START;
		return;
	}
	else if (process == PRO_SWITCH)
	{
		currentcount = timeGetTime();

		if (randcount < currentcount)
		{
			process = nextprocess;
			currentcount = 0;
		}
	}

	run();
}

void Client::run()
{
	fd_set read_flags, write_flags;
	struct timeval waitd;
	int sel;

	waitd.tv_sec = 0;
	waitd.tv_usec = 0;
	FD_ZERO(&read_flags);
	FD_ZERO(&write_flags);
	FD_SET(clisock, &read_flags);

	sel = select(clisock + 1, &read_flags, &write_flags, (fd_set*)0, &waitd);
	if (FD_ISSET(clisock, &read_flags))
	{
		FD_CLR(clisock, &read_flags);

		char buff[SOCKET_BUFFER];
		memset(&buff, 0, SOCKET_BUFFER);
		int recvsize = recv(clisock, buff, SOCKET_BUFFER, 0);
		if (recvsize > 0)
		{
			int buffpos = 0;
			while (recvsize > 0)
			{
				int copysize = recvbuffer.currentsize + recvsize <= SOCKET_BUFFER ? recvsize : SOCKET_BUFFER - recvbuffer.currentsize;
				memcpy(recvbuffer.buffer + recvbuffer.currentsize, buff+buffpos, copysize);
				buffpos += copysize;
				recvbuffer.currentsize += copysize;
				recvsize -= copysize;
				recvdone();
			}
		}
		else
		{
			Network::getInstance()->removeclient(this);
		}
	}

	// 보낼것이 있으면 보낸다로 설정
	if (sendbuffer.packetsize > 0 || !sendbufferlist.empty())
		FD_SET(clisock, &write_flags);

	// 보냄
	if (FD_ISSET(clisock, &write_flags))
	{
		FD_CLR(clisock, &write_flags);

		if (sendbuffer.currentsize == 0)
		{
			memcpy(sendbuffer.buffer, sendbufferlist[0].buffer, SOCKET_BUFFER);
			sendbuffer.currentsize = 0;
			sendbuffer.packetsize = sendbufferlist[0].packetsize;
			sendbufferlist.pop_front();
		}


		int sendsize = ::send(clisock, sendbuffer.buffer + sendbuffer.currentsize, sendbuffer.packetsize - sendbuffer.currentsize, 0);
		if (sendbuffer.packetsize == sendbuffer.currentsize + sendsize)
		{
			sendbuffer.packetsize = 0;
			sendbuffer.currentsize = 0;
			memset(sendbuffer.buffer, 0, SOCKET_BUFFER);
		}
		else
		{
			sendbuffer.currentsize += sendsize;
		}
	}


	// parse packet
	if (!recvbufferlist.empty())
	{
		for (int i = 0; i < recvbufferlist.size(); i++)
		{
			parsePacket(recvbufferlist[i]);
			
		}

		recvbufferlist.clear();
	}
}

void Client::parsePacket(SocketBuffer buf)
{
	PacketHeader* header = (PacketHeader*)buf.buffer;
	if (header->packetID == GameMsgID::MSG_ACK_CREATE_ACCOUNT)
	{
		printf("Created Account !\n");
		process = PRO_SWITCH;
		currentcount = timeGetTime();
		randcount = currentcount + (MINDELAY + (rand() % MAXDELAY));
		nextprocess = PRO_LOGIN;
	}
	else if (header->packetID == GameMsgID::MSG_ACK_LOGIN)
	{
		printf("Account Logined!\n");
		process = PRO_SWITCH;
		currentcount = timeGetTime();
		randcount = currentcount + (MINDELAY + (rand() % MAXDELAY));
		nextprocess = PRO_QUIT;
	}
}

void	Client::recvdone()
{
/*
	while (true)
	{
		if (recvbuffer.currentsize > sizeof(unsigned short))
		{
			// packet size는 packet size변수(unsigned short)를 포함하지 않는다
			unsigned short packetsize = *(unsigned short *)recvbuffer.buffer;
			if (recvbuffer.currentsize >= packetsize + sizeof(unsigned short))
			{
				SocketBuffer buffer;
				buffer.packetsize = packetsize;

				memcpy(buffer.buffer, recvbuffer.buffer, packetsize + sizeof(unsigned short));
				recvbufferlist.push_back(buffer);
				//buffer.debug();

				recvbuffer.currentsize -= packetsize+sizeof(unsigned short);

				if (recvbuffer.currentsize > 0)
				{
					char tempbuff[SOCKET_BUFFER];
					memcpy(tempbuff, recvbuffer.buffer + packetsize + sizeof(unsigned short), recvbuffer.currentsize);

					memset(recvbuffer.buffer, 0, SOCKET_BUFFER);
					memcpy(recvbuffer.buffer, tempbuff, recvbuffer.currentsize);

					//memmove(recvbuffer.buffer, recvbuffer.buffer + packetsize, recvbuffer.currentsize);
				}
			}
			else
				break;
		}
		else
			break;
	}
*/
	while (true)
	{
		if (recvbuffer.currentsize < sizeof(PacketHeader))
			break;

		PacketHeader* header = (PacketHeader*)recvbuffer.buffer;
		int packetsize = header->packetsize;

		if (packetsize > recvbuffer.currentsize)
			break;

		SocketBuffer buffer;
		buffer.packetsize = packetsize;

		memcpy(buffer.buffer, recvbuffer.buffer, packetsize + sizeof(unsigned short));
		recvbufferlist.push_back(buffer);

		recvbuffer.currentsize -= packetsize;

		if (recvbuffer.currentsize > 0)
		{
			char tempbuff[SOCKET_BUFFER];
			memcpy(tempbuff, recvbuffer.buffer + packetsize + sizeof(unsigned short), recvbuffer.currentsize);

			memset(recvbuffer.buffer, 0, SOCKET_BUFFER);
			memcpy(recvbuffer.buffer, tempbuff, recvbuffer.currentsize);

			//memmove(recvbuffer.buffer, recvbuffer.buffer + packetsize, recvbuffer.currentsize);
		}
	}

}

void Client::clearbuffer()
{
	recvbufferlist.clear();
	recvbuffer.reset();

	sendbufferlist.clear();
	sendbuffer.reset();
}

bool Client::sendpacket(SocketBuffer buffer)
{
	if (clisock == INVALID_SOCKET) return false;
	sendbufferlist.push_back(buffer);

	return true;
}


std::string Client::getString(int max_length)
{
	std::string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_int_distribution<> dist(0, possible_characters.size() - 1);
	std::string ret = "";
	for (int i = 0; i < max_length; i++) 
	{
		int random_index = dist(engine);
		ret += possible_characters[random_index];
	}
	return ret;
}