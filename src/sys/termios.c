
#include "sys/termios.h"



int tcsendbreak(int fd, int duration){
    return 0;
}

int tcdrain(int fd){
    return 0;
}

int tcflush(int fd, int queue_selector){
    return 0;
}

int tcflow(int fd, int action){
    return 0;
}

void cfmakeraw(struct termios *termios_p){
}

speed_t cfgetispeed(const struct termios *termios_p){
    return B115200;
}

speed_t cfgetospeed(const struct termios *termios_p){
    return B115200;
}

int cfsetispeed(struct termios *termios_p, speed_t speed){
    return 0;
}

int cfsetospeed(struct termios *termios_p, speed_t speed){
    return 0;
}

int cfsetspeed(struct termios *termios_p, speed_t speed){
    return 0;
}