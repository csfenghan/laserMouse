#include "location.hpp"
#include "mouse_control.hpp"
#include <pthread.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

lasermouse::Location location;    
lasermouse::MouseControl mouse_control;
pthread_mutex_t run_lock = PTHREAD_MUTEX_INITIALIZER;   // 通过这个互斥量控制检测线程

// 检测线程
void detect_and_control(int arg) {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s /dev/serial", argv[0]);
        exit(0);
    }

    int fd;
    char buf[1];

    fd = open(argv[1], O_RDONLY);
    pthread_mutex_lock(&run_lock);

    // 命令：
    //      c:标定
    //      s:启动检测
    //      S:停止检测
    //      l:点击左键
    //      r:点击右键
    while (read(fd, buf, 1) > 0) {
        if (buf[0] == 'c') {
            location.calibrate(); 
        } else if(buf[0] == 's') {
            pthread_mutex_unlock(&run_lock);
        } else if (buf[0] == 'S') {
            pthread_mutex_lock(&run_lock);
        } else if (buf[0] == 'l') {
            mouse_control.clickLeft();
        } else if (buf[0] == 'r') {
            mouse_control.clickRight();
        } else {
            
        }
    }
}
