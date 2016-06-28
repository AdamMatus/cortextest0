// Cortex serial communication handling on linux with termios
//

//C standard library
#include <stdio.h>

//C++ standard library
#include <iostream>
#include <string>

//POSIX terminal control definitions
#include <termios.h>

//OS specific definitions, linux/unix
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){

	int fd = 0; //filedescriptor
	struct termios config;

	const char *serial_device = "/dev/ttyUSB0";
	fd = open(serial_device, O_RDWR ); // O_read and write | not controlling terminal for process | O_use non blocking I/O )
	if(fd < 0) {
		perror( "failed to open serial port: " );
	}

	if(!isatty(fd)) {
		perror( "opened file is not tty: " );
	}
	
	if(tcgetattr(fd, &config) < 0) {
		perror("failed to get current serial line config\n");
	}

	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |INLCR | PARMRK | INPCK | ISTRIP | IXON);
	config.c_oflag = 0;
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	config.c_cflag &= ~(CSIZE | PARENB);
	config.c_cflag |= CS8;	
	config.c_cc[VMIN] = 1;
	config.c_cc[VTIME] = 0;

	if(cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
		perror("failed to setup in/out serial line speed\n");
	}

	if(tcsetattr(fd, TCSAFLUSH, &config) <0) {
		perror("failed to set termios struct to tty \n");
	}

	unsigned int char_num = 10; 
	char s[128];
	memset(s, 'x', sizeof(s));
	while(char_num--){
		if(read(fd, s, sizeof(s)) < 0)
			perror("read failed "); 
		else
			printf("%s",s);	
	}

	close(fd);	

	return 0;
}
