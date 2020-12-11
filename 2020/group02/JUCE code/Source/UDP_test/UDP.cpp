#include "UDP.h"


/***********************************************************************************************************************************************/
/******************** This file contains the implementation for UDP socket communication to the secondary device *******************************/
/***********************************************************************************************************************************************/

std::tuple<int,sockaddr_in> startSocketUDPSend() {
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(8888);
	inet_pton(AF_INET, "127.0.0.1", &si_other.sin_addr); //should be 127.0.0.1 for local loopback
	return { s,si_other };
}


void sendDataUDPJuce(SOCKET s, sockaddr_in si_other, const float* data, int device, int channel) {
	
	float dev = (float)device;
	float chan = (float)channel;

	char buf[lengthData*sizeof(float) + sizeof(float)];

	memcpy(buf,&dev,sizeof(float));
	memcpy(buf+sizeof(float), &chan, sizeof(float));
	memcpy(buf+2*sizeof(float), data, lengthData*sizeof(float));
	
	if (sendto(s,buf,lengthData * sizeof(float) + 2*sizeof(float), 0, (SOCKADDR *)&si_other, sizeof(si_other)) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}


void closeSocket(SOCKET s) {
	closesocket(s);
	WSACleanup();
}