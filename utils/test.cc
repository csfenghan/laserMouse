#include "serial_port.hpp"
#include <iostream>
#include <string.h>

#include <iconv.h>

int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
	iconv_t cd;
	char **pin = &str_str;
	char **pout = &dst_str;

	cd = iconv_open("utf8", "gbk");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
		return -1;
	iconv_close(cd);
	**pout = '\0';

	return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s {/dev/device}\n", argv[0]); 
        return -1;
    }
    laser_mouse::SerialPort com;
    com.open(argv[1], 9600);

    char buf[1024];
    char dst[1024];

    strcpy(buf, "AT?");    
    com.write(buf, 3);
    
    com.read(buf, 1024);
    GbkToUtf8(buf, strlen(buf), dst, 1024);
    std::cout << std::string(dst) << std::endl;
    
    com.close();
}
