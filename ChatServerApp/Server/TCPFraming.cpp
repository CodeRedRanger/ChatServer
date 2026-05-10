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

int TCPFraming::sendFrame(SOCKET s, const char* data, uint16_t length)
{


	if (length > 255)
		return PARAMETER_ERROR;

	uint8_t size = (uint8_t)length;

	int result = TCPFraming::tcp_send_whole(s, (char*)&size, 1);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	result = TCPFraming::tcp_send_whole(s, data, size);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	return SUCCESS;
}

int TCPFraming::readFrame(SOCKET s, char* buffer, uint16_t bufferSize)
{
	if (bufferSize > 256)
		return PARAMETER_ERROR;

	uint8_t size = 0;

	int result = TCPFraming::tcp_recv_whole(s, (char*)&size, 1);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	if (size >= bufferSize)
		return PARAMETER_ERROR;

	result = TCPFraming::tcp_recv_whole(s, buffer, size);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	buffer[size] = '\0';

	return SUCCESS;
}

