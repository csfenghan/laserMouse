#ifndef UTILS_SERIAL_PORT_HPP
#define UTILS_SERIAL_PORT_HPP

#include <termio.h>
#include <string>
#include "log4cpp/Category.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/FileAppender.hh"

namespace laser_mouse {
// 串口类
class SerialPort {
public:
    SerialPort();
    bool open(std::string dev, int speed=9600, int databits=8, int stopbits=1, 
            int parity='n', time_t timeout=10);

    ssize_t read(void *buf, size_t count);
    std::string read();
    ssize_t write(const void *buf, size_t count);
    void write(std::string buf);

    void close();
    void setSpeed(int speed);
    int readSpeed();
    bool spctl(int databits, int stopbits, int parity, time_t timeout = 10);

private:
    int fd_;
    log4cpp::Category &logger_;
};
}

#endif
