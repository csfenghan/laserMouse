/*
 * 本文件定义用到的摄像头和显示器等的配置信息
 * */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <exception>
#include <jsoncpp/json/json.h>

namespace lasermouse {
struct Config {
    Config():camera_source(-1), camera_height(-1), camera_width(-1),
            screen_height(-1), screen_width(-1) {}
    explicit Config(const std::string &path) {
        open(path);
    }

    /*
     * description: 读取json配置文件，加载配置信息
     * param:
     *    path:配置文件的路径
     * */
    void open(const std::string &path) {
        Json::Value root;
        Json::Reader reader;
        std::ifstream ifs(path);

        if (!reader.parse(ifs, root)) 
            throw std::invalid_argument("Error: jsoncpp parse error");
        
        camera_source = root["camera"]["source"].asInt();
        camera_width = root["camera"]["width"].asInt();
        camera_height = root["camera"]["height"].asInt();

        screen_width = root["screen"]["width"].asInt();
        screen_height = root["screen"]["height"].asInt();

        remoteControl_source = root["remoteControl_source"]["source"].asString(); 
    }
    int camera_source;
    int camera_height;
    int camera_width;
    
    int screen_height;
    int screen_width;

    std::string remoteControl_source;
};
}

#endif
