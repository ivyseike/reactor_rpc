#include <iostream>
#include <thread>
#include "ThreadPool.h"
#include <unistd.h>
#include <sys/fcntl.h>
#include "EventLoop.h"
#include "Channel.h"
#include "TimerFd.h"
#include "EventLoopThread.h"
#include "Acceptor.h"
#include <chrono>
#include "InetAddress.h"
#include "TcpServer.h"
#include "TcpClient.h"
#include "Codec.h"
#include "RPC_Server.h"

#include <signal.h>
#include <cstdlib>
#include <execinfo.h>

#include "configor/json.hpp"
#include "RPC_Client.h"
//#include "RPC_Client.h"

#define BACKTRACE_SIZE 16

void ShowStack(void)
{
    int i;
    void *buffer[BACKTRACE_SIZE];

    int n = backtrace(buffer, BACKTRACE_SIZE);
    printf("[%s]:[%d] n = %d\n", __func__, __LINE__, n);
    char **symbols = backtrace_symbols(buffer, n);
    if(NULL == symbols){
        perror("backtrace symbols");
        exit(EXIT_FAILURE);
    }
    printf("[%s]:[%d]\n", __func__, __LINE__);
    for (i = 0; i < n; i++) {
        printf("%d: %s\n", i, symbols[i]);
    }

    free(symbols);
}

void sigsegv_handler(int signo) {
    if (signo == SIGSEGV) {
        printf("Receive SIGSEGV signal\n");
        printf("-----call stack-----\n");
        ShowStack();
        exit(-1);
    } else {
        printf("this is sig %d", signo);
    }
}

void callfunc(){
    Socket my_sock(socket(PF_INET,SOCK_STREAM,0));
    InetAddress serv_addr("127.0.0.1", 9000);
    ::connect(my_sock.fd(), serv_addr.getSockAddr(), static_cast<socklen_t>(sizeof(*serv_addr.getSockAddr())));

    for(auto i = 0; i < 1; i++){
        auto write_bytes = ::write(my_sock.fd(), "A", 1);
        std::cout << "write: " << write_bytes << std::endl;
    }

}

void messageCallback(const std::shared_ptr<TcpConnection>& conn, const char* buf, int len){
    auto len_ = strlen(buf);
    if(len_ != len){
        for(int i = 0; i < len; i++){
            printf("%x\n", (unsigned char)buf[i]);
        }
    }
    else{
        std::string mess(buf);
        if(mess.size() != len) mess.resize(len);
        std::cout << "Received message from: " << conn->peerAddress().toIpPort()
                  << " " << mess << std::endl;
    }

}

void serverCode(int serverPort){
    EventLoop loop;
    InetAddress listenAddr(serverPort);
    TcpServer echo_server(&loop, listenAddr, "echo server");
    echo_server.setThreadNum(std::thread::hardware_concurrency());
    echo_server.setMessageCallback(messageCallback);
    echo_server.start();
    loop.loop();
}

void clientConnectionCallback(const std::shared_ptr<TcpConnection>& conn){
    std::cout << "Conenction established!" << std::endl;

    configor::json j;
    j["function_name"] = "add";
    j["arg1"] = 1;
    j["arg2"] = 2;

    std::string json_str = j.dump();
    unsigned int bytes = strlen(json_str.c_str());
    std::cout << bytes << std::endl;

//    std::string mess(header);

}

void clientMessageCallback(const std::shared_ptr<TcpConnection>& conn, const char* buf, int len){
    std::string mess(buf);
    if(mess.size() != len) mess.resize(len);
    std::cout << "Received message from: " << conn->peerAddress().toIpPort()
              << " " << mess << std::endl;

//    if(mess[0] != '9') {
//        char next = mess[0] + 1;
//        conn->send(std::string(10, next));
//    }
}

void clientCode(int serverPort){
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", serverPort);
    TcpClient echo_client(&loop, serverAddr, "echo client");
    echo_client.setConnectionCallback(clientConnectionCallback);
    echo_client.setMessageCallback(clientMessageCallback);
    echo_client.connect();
    loop.loop();

}



int main() {
    RPC_Client c("127.0.0.1", 9000);
//    Codec c_;
//    auto res = c_.pack_args("hello", 2, "abc");
    auto [res, success] = c.call<int>("hello",1);
    std::cout << success << std::endl;



//    std::string json_str = j.dump();
//    unsigned int bytes = strlen(json_str.c_str());
//    std::cout << bytes << std::endl;
//    char header[4]; // 4 bytes
//
//    header[0] = bytes >> 24 & 0xFF; //高位
//    header[1] = bytes >> 16 & 0xFF;
//    header[2] = bytes >> 8 & 0xFF;
//    header[3] = bytes & 0xFF; //低位
//    printf("%x %x %x %x\n", (unsigned char)header[0], (unsigned char)header[1], (unsigned char)header[2], (unsigned char)header[3]);
//
//    unsigned int recover = 0;
//    recover += header[0] << 24;
//    recover += header[1] << 16;
//    recover += header[2] << 8;
//    recover += header[3];

//    std::cout << recover << std::endl;
//    RPC_Server s(9000);
//    s.register_handler("clientCode", clientCode);



//    Codec c;
//    std::thread thread1(serverCode, 9000);
//    std::thread thread2(clientCode, 9000);




    return 0;
}
