#include "TCPFraming.h"


// loop recv func
int TCPFraming::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;

	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

// loop send func
int TCPFraming::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

void TCPFraming::sendFrame(SOCKET s, const char* data, uint16_t length)
{
	uint8_t size = (uint8_t)length;
	TCPFraming::tcp_send_whole(s, (char*)&size, 1);
	TCPFraming::tcp_send_whole(s, data, size);
}