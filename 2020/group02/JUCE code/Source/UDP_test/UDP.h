
#include<stdio.h>
#include<winsock2.h>
#include <Ws2tcpip.h>
#include <tuple>
#include <iostream>



#define lengthData 480

//void startAdvertiser();

//void startServiceSeeker();

std::tuple<int, sockaddr_in, sockaddr_in> startSocketUDPListen();

std::tuple <float*, int ,int> receiveDataUDPJuce(SOCKET s, sockaddr_in si_other);

std::tuple<int, sockaddr_in> startSocketUDPSend();

void sendDataUDPJuce(SOCKET s, sockaddr_in si_other, const float* bufferData, int device, int channel);

void closeSocket(SOCKET s);		// never run from the sender side 