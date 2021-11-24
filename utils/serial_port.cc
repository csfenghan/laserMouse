#include "serial_port.hpp"
#include "log4cpp/PatternLayout.hh"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>

namespace laser_mouse {
SerialPort::SerialPort():
    logger_(log4cpp::Category::getInstance("SerialPort")) 
{
    log4cpp::OstreamAppender *appender = new log4cpp::OstreamAppender("SerialPort", &std::cout);
    log4cpp::PatternLayout *pattern_layout = new log4cpp::PatternLayout();

    pattern_layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l} %t [%p] %m %n");
    appender->setLayout(pattern_layout);
    logger_.setAppender(appender);
    //logger_.setPriority(log4cpp::Priority::DEBUG);

    fd_ = -1;
}

bool SerialPort::open(std::string dev, int speed, int databits, int stopbits, int parity, time_t timeout) {
    if ((fd_ = ::open(dev.c_str(), O_RDWR | O_NOCTTY)) == -1) {
        logger_.error("Device %s open failed", dev);
        return false;
    }
    logger_.info("Successfully open serial port");

    setSpeed(speed);
    spctl(databits, stopbits, parity, timeout);

    return true;    
}

ssize_t SerialPort::read(void *buf, size_t count) {
    ssize_t n;
        
    if ((n = ::read(fd_, buf, count)) < 0)
        logger_.error("Serial port read error");
    return n;
}

std::string SerialPort::read() {
    
}

ssize_t SerialPort::write(const void *buf, size_t count) {
    ssize_t n;
    
    if ((n = ::write(fd_, buf, count)) < 0) 
        logger_.error("Serial port write error");
    
    return n;
}

void SerialPort::close() {
    ::close(fd_);
    fd_ = -1;
}

void SerialPort::setSpeed(int speed) {
    static std::unordered_map<int, speed_t> speed_to_param_ = {
        {2400, B2400}, {4800, B4800}, {9600, B9600}, {19200, B19200},
        {38400, B38400}, {57600, B57600}, {115200, B115200}
    };
    if (speed_to_param_.find(speed) == speed_to_param_.end())
        logger_.error("Unrecognized speed parameters");

    struct termios opt;
    tcgetattr(fd_, &opt);

    tcflush(fd_, TCIOFLUSH); 
    cfsetispeed(&opt, speed_to_param_[speed]);
    cfsetospeed(&opt, speed_to_param_[speed]);
    if (tcsetattr(fd_, TCSANOW, &opt) < 0)
        logger_.warn("Failed to set attribute of serial");
    tcflush(fd_, TCIOFLUSH); 
    
    logger_.info("Successfully set the port baud rate to %d", speed);
}

int SerialPort::readSpeed() {
    static std::unordered_map<int, speed_t> param_to_speed_ = {
        {B2400, 2400}, {B4800, 4800}, {B9600, 9600}, {B19200, 19200},
        {B38400, 38400}, {B57600, 57600}, {B115200, 115200}
    };
    struct termios opt;
    tcgetattr(fd_, &opt);
    
    speed_t speed = cfgetispeed(&opt);
    if (param_to_speed_.find(speed) == param_to_speed_.end()) {
        logger_.warn("Unknown baud rate");
        return -1;
    }
    return param_to_speed_[speed];
}

bool SerialPort::spctl(int databits, int stopbits, int parity, time_t timeout) {
    struct termios opt;
    tcgetattr(fd_, &opt);

    if (databits == 7) 
        opt.c_cflag |= CS7;
    else if (databits == 8)
        opt.c_cflag |= CS8;
    else {
        logger_.error("Invalid input parameter, does not suppoert databits equal to %d", databits);
        return false;
    }

    if (stopbits == 1) 
        opt.c_cflag &= ~CSTOPB;
    else if (stopbits == 2)
        opt.c_lflag |= CSTOPB;
    else {
        logger_.error("Invalid input parameter, does not support stopbits equal to %d", stopbits);
        return false;
    }

    if (parity == 'n' || parity == 'N') {
        opt.c_cflag &= ~PARENB; // Clear parity enable 
        opt.c_iflag &= ~INPCK; // Enable parity checking
    } else if (parity == 'e' || parity == 'E') {
        opt.c_cflag |= PARENB;     // Enable parity     
        opt.c_cflag &= ~PARODD;   // 转换为偶效验
        opt.c_iflag |= INPCK;       // Disnable parity checking 
    } else if (parity == 'e' || parity == 'E') {
        opt.c_cflag &= ~PARENB;
        opt.c_cflag &= ~CSTOPB; 
    } else {
        logger_.error("Invalid input parameter, does not support parity equl to %c", parity);
        return false;
    }

    // Set input parity option 
    if (parity != 'n' && parity != 'N')   
        opt.c_iflag |= INPCK; 
    tcflush(fd_, TCIFLUSH);

    // raw mode
    opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  // Input
    opt.c_oflag  &= ~OPOST;   // Output

    opt.c_cc[VTIME] = 10 * timeout; // 设置超时时间
    opt.c_cc[VMIN] = 0; // Update the opt and do it NOW 
    if (tcsetattr(fd_,TCSANOW,&opt) != 0) {
        logger_.error("Failed to set serial port properties");
        return false;
    }

    logger_.info("Successfully set the serial port databit to %d, the stopbit to %d, " 
            "parity mode to %c", databits, stopbits, parity);

    return true;
}

}

