#pragma once

#include <Windows.h>
#include <winsock.h>
#include <deque>
#include <string>


#define ENDOFPACKET			27	// esc
#define SOCKET_BUFFER		4096

/*
    |     ��Ŷ ���� ���� (���۵Ǵ� ����)          |
    |<--------------------------------------->|
	|                                         |
	+---+---+--------------      -------------+
	| 0 | 1 |               ....              |
	+---+---+--------------      -------------+
	|  (1)  |   (2) packet data               | 
	| 2byte |                                 |

	(1) unsigned short : packet ���� (2)��ŭ��
	(2) ���۵����� (1) ����Ʈ ��ŭ

*/


struct SocketBuffer
{
	unsigned short	packetsize;
	unsigned short	currentsize;
	char	buffer[SOCKET_BUFFER];
	SocketBuffer()
	{
		reset();
	}

	void reset()
	{
		packetsize = 0;
		currentsize = 0;
		memset(buffer, 0, SOCKET_BUFFER);
	}

	void debug()
	{
		printf("%s\n", buffer + sizeof(unsigned short));
	}
};

#define	PRO_WAIT				0
#define	PRO_START				1
#define	PRO_CREATE_ACCOUNT		2
#define PRO_LOGIN				3
#define PRO_QUIT				4
#define PRO_SWITCH				5



class Client
{
public :
	Client();
	~Client();

	void	init();
	void	uninit();
	void	processing();

	void	run();
	void	recvdone();
	void	clearbuffer();
	bool	sendpacket(SocketBuffer buffer);

	
	void parsePacket(SocketBuffer buf);
	std::string getString(int num);

	SOCKET clisock;

	std::deque<SocketBuffer>	recvbufferlist;
	std::deque<SocketBuffer>	sendbufferlist;
	SocketBuffer sendbuffer;
	SocketBuffer recvbuffer;

	int	process;
	int	nextprocess;

	std::string name;
	std::string pass;

	long randcount;
	long currentcount;
};
