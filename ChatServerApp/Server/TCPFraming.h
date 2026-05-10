#pragma once
#include <winsock2.h>
#include <cstdint>


namespace TCPFraming
{

	int tcp_recv_whole(SOCKET s, char* buf, int len);
	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
	void sendFrame(SOCKET s, const char* data, uint16_t length);
}