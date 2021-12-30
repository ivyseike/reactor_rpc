//
// Created by Yuchen Qin on 2021/10/11.
//

#ifndef TESTP_SOCKET_H
#define TESTP_SOCKET_H

#include <sys/socket.h>
class InetAddress;

class Socket{
public:
    explicit Socket(int sockfd): sockfd_(sockfd) { }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& localaddr);
    void listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr);
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setTcpKeepAlive(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);

private:
    const int sockfd_;

};


#endif //TESTP_SOCKET_H
