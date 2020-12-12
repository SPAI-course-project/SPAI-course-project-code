//
// Created by victor Letens on 28/11/20.
//

#ifndef SERVERUDP_SERVERUDP_H
#define SERVERUDP_SERVERUDP_H
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
struct audioBlock{
    int channel;
    float value[512]={};
};

class ServerUdp {

public:
    explicit ServerUdp(int PORT);
    void setup(int PORT);
    bool receive(audioBlock & x) const;
    bool closeSocket() const;

private:
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
};


#endif //SERVERUDP_SERVERUDP_H
