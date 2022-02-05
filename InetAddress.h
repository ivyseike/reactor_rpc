//
// Created by Yuchen Qin on 2021/10/11.
//

#ifndef TESTP_INETADDRESS_H
#define TESTP_INETADDRESS_H

#include <netinet/in.h>
#include <string>

//对sockaddr_in的包装
class InetAddress{
public:
    explicit InetAddress(uint16_t port = 0);
    InetAddress(std::string ip, uint16_t port);

    explicit InetAddress(const struct sockaddr_in& addr): addr_(addr) {}

    sa_family_t family() const { return addr_.sin_family;}
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

    const struct sockaddr* getSockAddr() const { return (struct sockaddr*)&addr_; }
    void setSockAddrIn(const struct sockaddr_in& addr){ addr_ = addr;}
    //static bool resolve(std::string hostname, InetAddress *result);

private:
    struct sockaddr_in addr_;
};

#endif //TESTP_INETADDRESS_H
