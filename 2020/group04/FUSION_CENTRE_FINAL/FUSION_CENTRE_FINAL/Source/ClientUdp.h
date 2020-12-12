//
// Created by victor Letens on 28/11/20.
// Rewritten for Windows on 10/12/20 by Thai Duong Truong
//

#ifndef CLIENTUDP_CLIENTUDP_H
#define CLIENTUDP_CLIENTUDP_H
#define _WINSOCKAPI_
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <iostream>

struct audioBlock {
    int channel;
    int availableSamples;
    float value[256] = {};
};

typedef struct audioBlock audioblock_t;

constexpr int LASTCHANNEL{ 999 };

class ClientUdp {
public:
    ClientUdp();
    ClientUdp(PCSTR ipAddress,int PORT_SERVER);
    bool setupClient(PCSTR ipAddress, int PORT_SERVER);
    bool sendFloat(float value);
    bool sendAudioBlock(audioblock_t& pAudioBlock);
private:
    SOCKET m_clientSocket;
    struct sockaddr_in m_clientAddr;
};


#endif //CLIENTUDP_CLIENTUDP_H
