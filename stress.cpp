
#include <iostream>
using namespace std;
#include <windows.h>
#include <process.h>
#include "stress.h"
#include "client.h"

#include <vector>


unsigned int __stdcall  ThreadEntryPointA(void *param)
{
	int maxclient = 200;
	vector<Client*> clientlist;

	// Connect
	for (int i=0; i< maxclient; i++)
	{
		Client *client = new Client();
		clientlist.push_back(client);
	}

	while (1)
	{
		for (int i=0; i< maxclient; i++)
		{
			clientlist[i]->processing();
			Sleep(1);
		}
	}

	return 0;
}


Stress::Stress()
{

}
Stress::~Stress()
{

}

void Stress::init()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	for (int i=0; i<20; i++)
	{
		_beginthreadex(NULL, 0, ThreadEntryPointA, (void *)i, 0, 0);
		Sleep(300);
	}

}

void Stress::run()
{
	   
}