//
// Created by Yuchen Qin on 2021/10/11.
//

#include "InetAddress.h"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>


InetAddress::InetAddress(uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(std::string ip, uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_aton(ip.c_str(), &addr_.sin_addr);
}

std::string InetAddress::toIp() const {
    char buff[20];
    char *str_ptr;
    str_ptr = inet_ntoa(addr_.sin_addr);
    strcpy(buff, str_ptr);
    return buff;
}

std::string InetAddress::toIpPort() const {
    char buff[20];
    char *str_ptr;
    str_ptr = inet_ntoa(addr_.sin_addr);
    strcpy(buff, str_ptr);
    return std::string(buff) + ":" +  std::to_string(ntohs(addr_.sin_port));
}

/*
bool InetAddress::resolve(std::string hostname, InetAddress* out)
{
    assert(out != nullptr);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    memset(&hent, 0, sizeof(hent));

    he = gethostbyname(hostname.c_str());
    for(auto i = 0; he->h_addr_list[i] != nullptr; i++){
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr_list[i]);
        return true;
    }
    std::cerr << "Error resolving..."  << std::endl;
    return false;

}
*/

