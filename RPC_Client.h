//
// Created by Yuchen Qin on 2021/11/15.
//

#ifndef TESTP_RPC_CLIENT_H
#define TESTP_RPC_CLIENT_H

#include "EventLoop.h"
#include "TcpClient.h"
#include "Codec.h"
#include "InetAddress.h"


class RPC_Client{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)> ;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const char* buffer, int len)>;

    RPC_Client(const RPC_Client&) = delete;
    RPC_Client& operator=(const RPC_Client&) = delete;
    RPC_Client(std::string ip, uint16_t port): serverAddr_(ip, port), loop_(new EventLoop),
                                                client_(loop_, serverAddr_, "RPC Client"),
                                                codec_(new Codec), read_bytes(0) {
        client_.setConnectionCallback(std::bind(&RPC_Client::onConnection, this, std::placeholders::_1));
//        client_.setMessageCallback(std::bind(&RPc_Client::parse_result, this, std::plac)
    }

    void connect(){
        client_.connect();
    }



    template<typename T, typename... Args>
    std::pair<typename std::enable_if<!std::is_void<T>::value, T>::type, bool> call(const std::string& func_name, Args&&... args){
        if(conn_ == nullptr){
            return std::make_pair(T(), false);
        }

        std::string packed_args = codec_->pack_args(func_name, std::forward<Args>(args)...);
        const char* bytes = codec_->cal_headers(packed_args);
        conn_->send(bytes, 4);
        conn_->send(packed_args);

        //等在条件变量上，当结果完全传输时再唤醒

        std::unique_lock<std::mutex> lk(m_);
        cond_.wait(lk, [this] { return read_bytes != 0 && read_bytes == readBuffer_.size(); });

       return codec_->template unpack_result<T>(readBuffer_);
    };

    void onConnection(const TcpConnectionPtr& conn){
        conn_ = conn;
    }

    void onMessage(const TcpConnectionPtr&, const char* buffer, int len){
        if(read_bytes == 0){
            if(len < 4){
                std::cerr << "..." << std::endl;
            }
            else{
                unsigned int bytes = 0;
                bytes += buffer[0] << 24;
                bytes += buffer[1] << 16;
                bytes += buffer[2] << 8;
                bytes += buffer[3];

                {
                    std::lock_guard<std::mutex> lk(m_);
                    read_bytes = bytes;
                    readBuffer_.append(buffer+4, buffer+len);
                }
            }
        }
        else{
            {
                std::lock_guard<std::mutex> lk(m_);
                readBuffer_.append(buffer, buffer+len);
            }
            if(readBuffer_.size() == read_bytes){
                cond_.notify_one();
            }
        }

    }


private:
    EventLoop* loop_;
    TcpClient client_;
    InetAddress serverAddr_;
    std::unique_ptr<Codec> codec_;
    TcpConnectionPtr conn_;

    std::mutex m_;
    std::condition_variable cond_;

    std::string readBuffer_;
    unsigned int read_bytes;
};
#endif //TESTP_RPC_CLIENT_H
