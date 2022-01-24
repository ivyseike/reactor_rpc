//
// Created by Yuchen Qin on 2021/11/4.
//

#ifndef TESTP_CODEC_H
#define TESTP_CODEC_H

#include "configor/json.hpp"
#include <string>
#include <iostream>

class Codec {
public:
    Codec(): cur_index(0) {}

    template <typename... Args>
    std::string pack_args(const std::string& funcName, Args&&... args){
        j = configor::json();
        cur_index = 0;

        j["function_name"] = funcName;
        if(sizeof...(args) > 0){
            std::initializer_list<int>{(pack_single_arg(args), 0)...};
        }

        return j.dump();
    }

    template <typename T>
    void pack_single_arg(T arg){
        j["arg" + std::to_string(cur_index++)] = arg;
    }

    template <typename... ArgTypes>
    std::pair<int, std::tuple<ArgTypes...>> unpack(std::string s){
        configor::json j_ = configor::json::parse(s);

        std::tuple<ArgTypes...> arg_tuple; //final result
        std::string func_name;
        int index = 0;
        for (auto iter = j.begin(); iter != j.end(); iter++ ) {
            try{
                if(iter.key() != "function_name"){
                    std::get<index>(arg_tuple) = iter.value();
                    index++;
                }
                else{
                    func_name = iter.key();

                }
            }
            catch(...){
                std::cerr << "Error when processing arguments..." << std::endl;
                return std::make_pair(-1, arg_tuple);
            }
        }

        return std::make_pair(0, arg_tuple);
    }

    template <typename T>
    std::string pack_result(T result, bool success){
        configor::json j_;
        j_["result"] = result;
        j_["success"] = true;
        return j_.dump();
    }

    std::string unpack_func_name(const std::string& s){
        configor::json j_ = configor::json::parse(s);
        return j_["func_name"].is_null() == 1 ? "" : j_["func_name"];
    }

    const char* cal_headers(const std::string& s){
        unsigned int bytes = s.size();
        char* header = new char[4];

        header[0] = bytes >> 24 & 0xFF; //高位
        header[1] = bytes >> 16 & 0xFF;
        header[2] = bytes >> 8 & 0xFF;
        header[3] = bytes & 0xFF; //低位
        //printf("%x %x %x %x\n", (unsigned char)header[0], (unsigned char)header[1], (unsigned char)header[2], (unsigned char)header[3]);
        return header;
    }

    template<typename T>
    std::pair<T, bool> unpack_result(const std::string& s){
        configor::json j_ = configor::json::parse(s);
        bool success = false;
        if(j_["success"].try_get(success)){
            if(success){
                if(!j_["result"].is_null()){
                    try{
                        return std::make_pair(j_["result"], true);
                    }
                    catch(...){
                        return std::make_pair(T(), false);
                    }
                }
            }
        }
        return std::make_pair(T(), false);
    }

private:
    configor::json j;
    int cur_index;


};


#endif //TESTP_CODEC_H
