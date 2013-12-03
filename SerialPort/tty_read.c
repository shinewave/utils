/*
 * tty_read.c
 * read form linux tty device
 *
 * shinewave<shinewave-c@163.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define UART_DEV "/dev/ttyS5"

int uart_fd;

int main()
{
        struct termios oterm_attr, term_attr;
        unsigned char buf[256];
        int ret;
        unsigned int baud;

        baud = B115200;

        /* init UART for SOL*/
        uart_fd = open(UART_DEV, O_RDWR | O_NONBLOCK | O_NOCTTY, 0);
        if(uart_fd < 0){
                printf("Couldn't open <%s>\n", UART_DEV);
                return 1;
        }
        tcgetattr(uart_fd, &oterm_attr);

        bzero(&term_attr, sizeof(struct termios));
        term_attr.c_iflag &= ~(IXON | IXOFF | IXANY);
        term_attr.c_oflag &= ~(OPOST | ONLCR | OCRNL);
        term_attr.c_lflag &= ~(ISIG | ECHO | ECHOE | ICANON);

        term_attr.c_cflag |= (CLOCAL | CREAD);
        term_attr.c_cflag &= ~CBAUD;
        term_attr.c_cflag |= baud; /* baud */
        term_attr.c_cflag &= ~(CSIZE); /* databits */
        term_attr.c_cflag |= CS8;
        term_attr.c_cflag &= ~(PARENB); /* no parity */
        term_attr.c_cflag &= ~CSTOPB; /* 1 stopbits */
        term_attr.c_cflag &= ~CRTSCTS;
        //term_attr.c_cflag |= CRTSCTS; /* enable hardware flow control */
        term_attr.c_cc[VMIN] = 0;
        term_attr.c_cc[VTIME] = 10;

        printf("cflag: %x, size: %d\n", term_attr.c_cflag, sizeof(term_attr.c_cflag));

        cfsetispeed(&term_attr, baud);
        cfsetospeed(&term_attr, baud);

        tcflush(uart_fd, TCIOFLUSH);
        if(tcsetattr(uart_fd, TCSANOW, &term_attr) < 0){
                printf("Fail to initialize <%s>\n", UART_DEV);
                return 1;
        }

        while (1){
                ret = read(uart_fd, buf, 256);
                if(ret > 0){
                        printf("%s", buf);
                }
        }

        return 0;
}
