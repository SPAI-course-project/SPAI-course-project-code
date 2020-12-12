//
// Created by victor Letens on 28/11/20.
// Rewritten for Windows on 10/12/20 by Thai Duong Truong
//

#define _WINSOCKAPI_
#include "ClientUdp.h"
#include <algorithm>
#include "plog/Log.h"

ClientUdp::ClientUdp()
{
    m_clientSocket = INVALID_SOCKET;
}

ClientUdp::ClientUdp(PCSTR ipAddress, int PORT) {
    std::cout <<"Starting client!\n";
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    int clientAddrSize{ sizeof(m_clientAddr) };
    if (res != NO_ERROR) {
        std::cout << "WSAStartup failed with error " <<  WSAGetLastError << "\n";
        exit(EXIT_FAILURE);
    }
    m_clientSocket = INVALID_SOCKET;
    m_clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_clientSocket == INVALID_SOCKET) {
        std::cout << "Failed to create client socket with error " << WSAGetLastError() << "\n";
        exit(EXIT_FAILURE);
    }

    m_clientAddr.sin_family = AF_INET;
    m_clientAddr.sin_port = htons(PORT);
    InetPton(AF_INET, ipAddress, &m_clientAddr.sin_addr.s_addr);
}
bool ClientUdp::setupClient(PCSTR ipAddress, int PORT_SERVER)
{
    LOGD << "Client UDP: Starting client!";
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    int clientAddrSize{ sizeof(m_clientAddr) };
    if (res != NO_ERROR) {
        LOGD << "Client UDP: WSAStartup failed with error " << WSAGetLastError;
        return false;
    }
    m_clientSocket = INVALID_SOCKET;
    m_clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_clientSocket == INVALID_SOCKET) {
        LOGD << "Failed to create client socket with error " << WSAGetLastError();
        return false;
    }

    m_clientAddr.sin_family = AF_INET;
    m_clientAddr.sin_port = htons(PORT_SERVER);
    if (InetPton(AF_INET, ipAddress, &m_clientAddr.sin_addr.s_addr) != 1) {
        LOGD << "Client UDP: InetPton failed!";
    }
    return true;
}

bool ClientUdp::sendFloat(float value) {
    char sendBuf[1024];
    int bufLen = (int)(sizeof(sendBuf) - 1);
    float* floatBuffer = (float*)sendBuf;
    *sendBuf = value;
    std::cout << "Client: sending float = " << value << "\n";
    int result{ sendto(m_clientSocket, sendBuf, bufLen, 0, (SOCKADDR*)&m_clientAddr, (int) sizeof(m_clientAddr)) };
    if (result < 0) {
        std::cout << "Failed to send float!\n";
        return false;
    }
    return true;
}

bool ClientUdp::sendAudioBlock(audioblock_t& pAudioBlock)
{
    char sendBuf[sizeof(audioblock_t) + 1];
    int bufLen = (int)(sizeof(sendBuf) - 1);
    audioblock_t* audioBlockBuffer{ (audioblock_t*) sendBuf };
    *audioBlockBuffer = pAudioBlock;
    /*
    audioBlockBuffer->availableSamples = pAudioBlock-> availableSamples;
    std::cout << "Client: audio block with = " << audioBlockBuffer->availableSamples << " available samples.\n";
    audioBlockBuffer->channel = pAudioBlock->channel;
    for (int i = 0; i < pAudioBlock->availableSamples; i++) {
        if (i < ((sizeof(pAudioBlock->value)) / sizeof(pAudioBlock->value[0]))) {
            audioBlockBuffer->value[i] = pAudioBlock->value[i];
            std::cout << "Client: sample " << i << " = " << audioBlockBuffer->value[i] << "\n";
        }
        
    }*/
    int result{ sendto(m_clientSocket, sendBuf, bufLen, 0, (SOCKADDR*)&m_clientAddr, (int)sizeof(m_clientAddr)) };
    if (result < 0) {
        std::cout << "Failed to send float!\n";
        return false;
    }
    
    return true;
}
