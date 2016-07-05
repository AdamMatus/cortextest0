// Cortex serial communication handling on linux with termios
// 
// DEBUG: ADD TO EXIT FUN TO RESTORE DEFAULT STDIO CONFIG

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


int tty_init(); // configuration of USBtty0 serial port
void stdio_init(); // configuration of stdio terinal
int write_line(int fd, char *str);

int main(int argc, char *argv[]){

	int fd = tty_init();
	stdio_init();

	int i = 0, j = 0;
	char s[128];
	char sin[128];
	char c = 0;
	int n = 0;
	memset(s, 0, sizeof s);
	memset(sin, 0, sizeof sin);
	
	char test_trans_string[] = "THIS IS a SAMPLE TEXT";
	
	printf("%s%d%s","sizeof ",(int)sizeof test_trans_string,"\n");

	while(1){

		if( read(fd, &c, sizeof(c)) < 0 ){
			perror("read from serial port failed "); 
			break;
		}
		else 
			s[i++] = c;

		if(c=='\n') {
			i = 0;
			printf("%s",s);
		}	

		if( (n = read(STDIN_FILENO, &c, sizeof c)) < 0 ){
			if( errno == EAGAIN )
				continue; // no data to be read
			else
		  	perror("cannot read from stdin ");	
		}
		else if(n > 0){
			sin[j++] = c;
			if(c=='\n'){
				sin[j-1] = 0;
				j = 0;
				if( write_line(fd, sin) <0 )
					perror("Failed to send a line ");
				else
					printf("%s%s%s","The message: \"",sin,"\" has been sent to the target.\n");
			}
		}
	}

	close(fd);	

	return 0;
}

int write_line(int fd, char *str){
	char *cp = str;  // xd
	while(*cp){
		if( write(fd, cp, sizeof *cp) <0 ){
			return -1;
		}
		else 
			cp++;
	}
	write(fd, cp, sizeof *cp); //sending 0

	return 0;
}	

int tty_init(){

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
	
	return fd;
}

void stdio_init(){
	struct termios config;
	if( tcgetattr(STDIN_FILENO, &config) <0)
		perror("cannot read STDIN termios config ");

	config.c_lflag &= ~(ECHO);
//	config.c_lflag |= ECHONL;

	if( tcsetattr(STDIN_FILENO, TCSANOW, &config) <0 )
		perror("cannot set stdin termios config ");

// no blocking on stdin	
	int flags =  fcntl(STDIN_FILENO, F_GETFL, 0);
	if( fcntl(STDIN_FILENO, F_SETFL, flags | FNDELAY) <0 )
		perror("failed to change stdin atrb to FNDELAY ");
}

