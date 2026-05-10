#include "TCPFraming.h"


// loop recv func
int TCPFraming::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	//bytes received so far
	int total = 0;

	do
	{
		//buffer offset by total bytes received, and length reduced by total bytes received, so that we only receive the remaining bytes needed to complete the frame
		int ret = recv(s, buf + total, len - total, 0);

		//for connection closed (= 0) or error (< 0), return the result to be handled by caller, otherwise add bytes received to total and keep looping
		if (ret < 1)
			return ret;
		else
			total += ret;
		//keep looping until we have received the full frame (total bytes received is equal to the length of the frame)
	} while (total < len);

	return total;
}

// loop send func
int TCPFraming::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	//keep sending until we have sent the full frame (bytes sent is equal to the length of the frame), or until an error occurs (result <= 0)
	while (bytesSent < length)
	{
		//send remaining bytes only
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		//for connection closed (= 0) or error (< 0), return the result to be handled by caller, otherwise add bytes sent to total and keep looping
		if (result <= 0)
			return result;

		//add bytes sent to total and keep looping until we have sent the full frame (bytes sent is equal to the length of the frame)
		bytesSent += result;
	}

	//if we exit the loop, it means we have sent the full frame successfully, so we return the total bytes sent (which should be equal to the length of the frame)
	//if 0 or negative was returned from send, it would have been returned immediately and we would not reach this point
	return bytesSent;
}

//includes the length of message, used for error checking, and passed to tcp_send_whole to ensure the entire frame is sent
int TCPFraming::sendFrame(SOCKET s, const char* data, uint16_t length)
{

	//because size stored in one byte, max length is 255, so if length exceeds this, return parameter error to be handled by caller
	if (length > 255)
		return PARAMETER_ERROR;

	uint8_t size = (uint8_t)length;

	//send the size first
	//errors include connection closed (= 0) or error (< 0), which are returned to be handled by caller, otherwise we proceed to send the body of the message
	int result = TCPFraming::tcp_send_whole(s, (char*)&size, 1);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	//send the body of message
	//erros same as above, but if successful, we return SUCCESS to indicate that the frame was sent successfully
	result = TCPFraming::tcp_send_whole(s, data, size);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	return SUCCESS;
}

int TCPFraming::readFrame(SOCKET s, char* buffer, uint16_t bufferSize)
{
	//allows for extra byte for message size + message body (255 + 1)
	//this is the total/max buffer size passed to function that can be  used to read a frame
	if (bufferSize > 256)
		return PARAMETER_ERROR;

	uint8_t size = 0;

	//read the size byte first, which tells us how many bytes to expect for the body of the message.
	//error handling as in sendFrame
	// plus prevent buffer overflow by checking if the size of actual message exceeds the buffer size provided by caller, and if so, return parameter error to be handled by caller
	//after call below, size is the length of the actual message that was sent. 
	int result = TCPFraming::tcp_recv_whole(s, (char*)&size, 1);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	if (size >= bufferSize)
		return PARAMETER_ERROR;

	//read full message
	//you know the size of the actual message (size) is less than the max buffer size, so now you will fill your buffer with the body of the message. 
	result = TCPFraming::tcp_recv_whole(s, buffer, size);

	if (result == 0)
		return SHUTDOWN;

	if (result < 0)
		return DISCONNECT;

	//add null terminate so can print the buffer
	buffer[size] = '\0';

	return SUCCESS;
}

