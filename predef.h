
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
	unsigned char	signature;		// 서버가 보낸것?(SERVER_SIG) 클라이언트가 보낸것? (CLIENT_SIG)
	int		packetsize;		// packet size는 header + data 포함한 전체 길이
	int		packetserial;	// 생성 번호 = wCommandID^dwSize+index(패키지당 자동 성장 색인); 환원 번호 = pHeader->dwPacketNo - pHeader->wCommandID^pHeader->dwSize;
	int		packetID;			// msg ID
};

#pragma  pack(pop)
