//
// Created by Yuchen Qin on 2021/11/4.
//

#ifndef TESTP_RPC_SERVER_H
#define TESTP_RPC_SERVER_H

#include "Codec.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include <thread>
#include <unordered_map>
#include "traits.h"


class RPC_Server{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)> ;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const char* buf, int len)> ;

    explicit RPC_Server(uint16_t port, int thread_num = std::thread::hardware_concurrency()):
            loop_(new EventLoop()),
            listenAddr_(port),
            server_(new TcpServer(loop_, listenAddr_, "RPC_Server", true)),
            codec_(new Codec())
    {
        server_->setThreadNum(thread_num);
//        server_->setConnectionCallback(onConnection);
        server_->setMessageCallback(std::bind(&RPC_Server::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    RPC_Server() = delete;
    RPC_Server(const RPC_Server&) = delete;
    RPC_Server& operator=(const RPC_Server&) = delete;

    void onMessage(const TcpConnectionPtr& conn, const char* buf, int len){
        auto iter = readBuffers_.find(conn);
        if(iter == readBuffers_.end()){
            if(len < 4){
                std::cerr << "..." << std::endl;
            }
            else{
                unsigned int bytes = 0;
                bytes += buf[0] << 24;
                bytes += buf[1] << 16;
                bytes += buf[2] << 8;
                bytes += buf[3];

                read_bytes[conn]  = bytes;
                readBuffers_[conn].append(buf+4, buf+len);
            }
        }
        else{
            readBuffers_[conn].append(buf, buf+len);
        }

        if(readBuffers_[conn].size() == read_bytes[conn]){
            handle_request(conn);
        }
    }

    void handle_request(const TcpConnectionPtr& conn){
        auto func_name = codec_->unpack_func_name(readBuffers_[conn]);
        auto iter = registered_functions.find(func_name);
        if(iter == registered_functions.end()){
            send_result(conn, "Function name unregistered", false);
        }
        else{
            registered_functions[func_name](readBuffers_[conn], conn);
        }
    }

    void start(){
        server_->start();
        loop_->loop();
    }

    template<typename F>
    void call_proxy(F func, std::string args, const TcpConnectionPtr& conn){
        using args_type = typename function_traits<F>::tuple_type;

        auto [res, arg_tuple] = codec_->unpack<args_type>(args);
        if(res == 0){
            /*
            if(test_void(func)){
                std::apply(func, arg_tuple);
                send_result(conn, "Function successfully called");
            }
            else{
                */
            auto func_res = std::apply(func, arg_tuple);
            //auto func_res = call(func, arg_tuple);
            send_result(conn, func_res);
        }
        else{
            send_result(conn, "Parameter mismatch", false);
            //send_error_msg(conn, "Parameter mismatch");
        }
    }

    template<typename T>
    void send_result(const TcpConnectionPtr& conn, T res, bool success = true){ //需要增加加包头的部分
         auto msg = codec_->pack_result(res, success);
         const char* headers = codec_->cal_headers(msg);
         conn->send(headers, 4);
         conn->send(msg);
    }

    template <typename FuncType>
    void register_handler(const std::string& func_name, const FuncType& f){
        registered_functions[func_name] = std::bind(&RPC_Server::call_proxy<FuncType>, this, f, std::placeholders::_1, std::placeholders::_2);
    }

    template<typename Function, typename Tuple, size_t ... I>
    auto call(Function f, Tuple t, std::index_sequence<I ...>){
        return f(std::get<I>(t) ...);
    }

    template<typename Function, typename Tuple>
    auto call(Function f, Tuple t){
        //auto res = std::apply(f, t);
        static constexpr auto size = std::tuple_size<Tuple>::value;
        return call(f, t, std::make_index_sequence<size>{});
    }

private:
    EventLoop* loop_;
    InetAddress listenAddr_;
    std::unique_ptr<TcpServer> server_;
    std::unordered_map<std::string, std::function<void(std::string, const TcpConnectionPtr&)>> registered_functions;
    std::unique_ptr<Codec> codec_;

    std::unordered_map<TcpConnectionPtr, std::string> readBuffers_;
    std::unordered_map<TcpConnectionPtr, unsigned int> read_bytes;

};


#endif //TESTP_RPC_SERVER_H
