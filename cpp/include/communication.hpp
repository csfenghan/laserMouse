/*
 * 本文件定义了与遥控器通信的接口及其相关的命令
 * */

#ifndef COMMUNICATION_HAPP
#define COMMUNICATION_HAPP

#include <string>
#include <fcntl.h>
#include "config.hpp"
#include <unistd.h>
#include <string.h>

namespace lasermouse {
enum Command {
    UNDEFINE = 0,
    CALIBRATE = 1,
    START = 2,
    STOP = 3,
    CLICK_LEFT = 4,
    CLICK_RIGHT= 5
};
class Communication {
public:
    Communication():path_(std::string()), fd_(0) {}
    explicit Communication(const std::string& path):path_(path), fd_(0) {}

    /*
     * description: 配置参数
     * */
    void setupConfig(const Config& conf) {
        path_ = conf.remoteControl_source;
    }

    /*
     * description: 初始化
     * */
    void init() {
        if (path_.empty())
            fd_ = STDIN_FILENO;
        else {
            char *ptr = new char[path_.size()];     // ps:直接用path_.c_str()的话cppcheck会报错，不知道什么鬼

            strncpy(ptr, path_.c_str(), path_.size());    
            fd_ = open(ptr, O_RDWR);

            delete [] ptr;
        }
    }

    /*
     * description: 读取命令
     * return: enum Command类型
     * */
    Command read_command() {
        char buf[1];
        Command result = UNDEFINE;

        if (read(fd_, buf, 1) > 0) {
            switch (buf[0]) {
                case 'c' : result = CALIBRATE; break;
                case 's' : result = START; break;
                case 'S' : result = STOP; break;
                case 'l' : result = CLICK_LEFT; break;
                case 'r' : result = CLICK_RIGHT; break;
                default : result = UNDEFINE; 
            }
        }

        return result;
    }
private:
    std::string path_; 
    int fd_;
};
}

#endif
