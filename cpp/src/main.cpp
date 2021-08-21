#include "location.hpp"
#include "mouse_control.hpp"
#include <pthread.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "config.hpp"
#include "communication.hpp"

lasermouse::Location location;          // 激光点定位
lasermouse::MouseControl mouse_control; // 鼠标控制

pthread_mutex_t run_lock;               // 通过这个互斥量控制检测线程
void *detect_and_control(void *);

int main(int argc, char **argv) {
    pthread_t pd;
    bool is_run = false;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <config.json>", argv[0]);
        exit(0);
    }
    // 初始化
    lasermouse::Config config;      // 读取配置文件
    lasermouse::Communication comm; // 读取命令

    config.open(argv[1]);
    location.setupConfig(config);
    location.init();
    comm.setupConfig(config);
    comm.init();

    // 初始化时持有锁，即默认检测线程阻塞
    pthread_mutex_lock(&run_lock);
    pthread_create(&pd, NULL, detect_and_control, NULL);     
    
    // 运行
    while (true) {
        auto command = comm.read_command();

        if (command == lasermouse::CALIBRATE) 
            location.calibrate();
        else if (command == lasermouse::START) {
            if (!is_run) {
                std::cout << "开始运行" << std::endl;
                pthread_mutex_unlock(&run_lock);
            }
            is_run = true;
        }
        else if (command == lasermouse::STOP) {
            if (is_run) {
                std::cout << "停止运行" << std::endl;
                pthread_mutex_lock(&run_lock);
            }
            is_run = false;
        }
        else if (command == lasermouse::CLICK_LEFT) {}
        else if (command == lasermouse::CLICK_RIGHT) {}
    }
}

// 检测线程
void *detect_and_control(void *arg) {
    int x = 0, y = 0;
    location.calibrate();

    while (true) {
        // 运行前检查是否可以获取锁，如果可以，说明当前可以运行；否则阻塞休眠
        pthread_mutex_lock(&run_lock);
        pthread_mutex_unlock(&run_lock);

        if (location.position(x, y)) 
            mouse_control.moveTo(x, y);
    }
}
