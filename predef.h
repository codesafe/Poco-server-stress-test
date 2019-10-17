
#pragma once


// google protobuf
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace google;

// game protobuf
#include "./pb/packet_def.pb.h"
#include "./pb/packet.pb.h"


#ifdef _DEBUG
#pragma  comment(lib,"libprotobufd.lib")
#else
#pragma  comment(lib,"libprotobuf.lib")
#endif

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")



#define CLIENT_SIG 0xC0
#define SERVER_SIG 0xDE

#pragma  pack(push)
#pragma  pack(1)


struct PacketHeader
{
	unsigned char	signature;		// ������ ������?(SERVER_SIG) Ŭ���̾�Ʈ�� ������? (CLIENT_SIG)
	int		packetsize;		// packet size�� header + data ������ ��ü ����
	int		packetserial;	// ���� ��ȣ = wCommandID^dwSize+index(��Ű���� �ڵ� ���� ����); ȯ�� ��ȣ = pHeader->dwPacketNo - pHeader->wCommandID^pHeader->dwSize;
	int		packetID;			// msg ID
};

#pragma  pack(pop)
