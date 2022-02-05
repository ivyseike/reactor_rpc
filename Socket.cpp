//
// Created by Yuchen Qin on 2021/10/11.
//

#include "Socket.h"
#include "InetAddress.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

void setNonBlockAndCloseOnExec(int sockfd){
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);

    (void)ret;
}

Socket::~Socket() {
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr) {
    int res = bind(sockfd_, localaddr.getSockAddr(), static_cast<socklen_t>(sizeof(*localaddr.getSockAddr())));
    if(res < 0)
        std::cerr << "Bind error" << std::endl;
}

void Socket::listen(){
    int res = ::listen(sockfd_, 100);
    if(res < 0)
        std::cerr << "Listen error" << std::endl;
}

int Socket::accept(InetAddress *peeraddr) {
    struct sockaddr_in addr;
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    memset(&addr, 0, sizeof(addr));
    int connfd = ::accept(sockfd_, (sockaddr*)&addr, &addrlen);
    if(connfd < 0){
        std::cerr << "Accept error" << std::endl;
        return -1;
    }
    else{
        setNonBlockAndCloseOnExec(connfd);
        peeraddr->setSockAddrIn(addr);
        return connfd;
    }
}

void Socket::shutdownWrite() {
    auto res = ::shutdown(sockfd_, SHUT_WR);
    if(res == -1) std::cerr << "Error when shutting down" << std::endl;
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setTcpKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on){
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if(ret < 0 && on){
        std::cerr << "Reuse port failed" << std::endl;
    }
#else
    if(on){
        std::cerr << "SO_REUSEPORT is not supported." << std::endl;
    }
#endif
}


