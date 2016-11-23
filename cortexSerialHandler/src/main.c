// Cortex serial communication handling on linux with termios
// 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <termios.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//***********************************************
//**************************************************
struct print_unit;

struct print_unit* last_pu;

struct print_unit{
	static const size_t mesg_max_length  = 64;
	static const size_t mesg_max_num = 10;

	size_t mesg_num;
	char *tail_mesgs[mesg_max_num];
	size_t next_print_index;

	const char *type_pattern;

	int fildes;
	const char* filename;

	struct print_unit* next;
};

size_t s_for_cmd(const char* mesg, const char* cmd_pattern){
	const char* cmd_p = cmd_pattern;
	const char* pattern_p = mesg; 
	size_t pattern_index = 0;

	while( *(cmd_p) ){
		
		switch(*cmd_p){
			case '.':
				pattern_index++;
				cmd_p++;
				break;

			case '+':
				if( *(cmd_p + 1) == pattern_p[pattern_index] ){
					pattern_index++;
					cmd_p+=2;
				}
				else 
					cmd_p--;
				break;

			default:
				if( *(cmd_p) == pattern_p[pattern_index] ){
					pattern_index++;
					cmd_p++;
				} else if( *(pattern_p + pattern_index) )
					return 0;
				else
					cmd_p = cmd_pattern;
				}
		}	
	return pattern_index;
}

size_t increment_mesg_index(struct print_unit* pu){
	size_t old = pu->next_print_index;
	pu->next_print_index = (pu->next_print_index + 1) %  pu->mesg_num;
	return old;
}

struct print_unit* parse_mesg(const char* mesg){
	struct print_unit* ppu = last_pu;
	while(ppu){
		if( s_for_cmd(mesg, ppu->type_pattern) > 0 ){
			strncpy(ppu->tail_mesgs[ppu->next_print_index], mesg, ppu->mesg_max_length);
			size_t len = strlen(ppu->tail_mesgs[ppu->next_print_index]);
			write(ppu->fildes, ppu->tail_mesgs[ppu->next_print_index], len);	
			increment_mesg_index(ppu);
			return ppu;
		}
		ppu = ppu->next;	
	}
	return NULL;
}

void print_mesgs(){
	struct print_unit* ppu = last_pu;
	while(ppu){
		for(size_t i = 0; i < ppu->mesg_num; i++){
			size_t len = strlen(ppu->tail_mesgs[i]);
			write(STDOUT_FILENO, ppu->tail_mesgs[i], len);
		}
		ppu = ppu->next;
	}
}

int add_to_list(struct print_unit* pu){
	if(pu == NULL){
		errno = EINVAL;
		return -1;
	}
	else if(last_pu == NULL){
		last_pu = pu;		
		return 0;
	}
	else{
		pu->next = last_pu;
		last_pu = pu;
		return 0;
	}
}

int init_print_unit(struct print_unit* punit,
	 									size_t mn,
									 	const char* pattern,
										const char* filename)
{
	if(mn < 1 || mn > print_unit::mesg_max_num){
		errno = EINVAL;
		return -1;
	}

	punit->type_pattern = pattern;	
	punit->mesg_num = mn;	

	for(unsigned int i=0; i < punit->mesg_num; i++){
		if(	(punit->tail_mesgs[i] = (char*)calloc(print_unit::mesg_max_length, sizeof(char))) < 0){
			perror("calloc error :");
			return -1;
		}
	}

	punit->next_print_index = 0;

	punit->filename = filename;

	if( (punit->fildes = open(punit->filename, O_WRONLY + O_APPEND + O_CREAT + O_TRUNC, S_IRWXU | S_IRWXO)) < 0){
		return -1;
	}

	return 0;
}

void clean_print_unit(struct print_unit* pu){
	for(unsigned int i=0; i < pu->mesg_num; i++){
		free(pu->tail_mesgs[i]);
		close(pu->fildes);
	}
}
//************************************************
//print units
static struct print_unit gyro_print;

void cleanup(){
	clean_print_unit(&gyro_print);
}
//**************************************************



int tty_init(); // configuration of USBtty0 serial port
void stdio_init(); // configuration of stdio terinal
int write_line(int fd, char *str);

int main(int argc, char *argv[]){

	atexit(&cleanup);

	if(	init_print_unit(&gyro_print, 3, ".+:.+:.+ GYRO", "./log/GYRO.txt") < 0){
		perror("init print unit failed: ");
		exit(EXIT_FAILURE);
	}

	add_to_list(&gyro_print);

	int fd = tty_init();
	stdio_init();

	int i = 0, j = 0;
	char s[128];
	char sin[128];
	char c = 0;
	int n = 0;
	memset(s, 0, sizeof s);
	memset(sin, 0, sizeof sin);
	
	while(1){

		if( (n = read(fd, &c, sizeof(c))) < 0 ){
			if( !(errno == EAGAIN) ){
				perror("read from serial port failed "); 
				break;
			}
		}
		else{ 
			s[i++] = c;
			if(c==0) {
				i = 0;
				struct print_unit *pu;
				if(!(pu = parse_mesg(s)))
					printf("%s",s);
				else if( pu->next_print_index == 0 )
					print_mesgs();
			}

			if( (n = read(STDIN_FILENO, &c, sizeof c)) < 0 ){
				if( !(errno == EAGAIN) )
					perror("cannot read from stdin ");	
			}

			else {
				sin[j++] = c;
				if(c=='\n'){
					sin[j-1] = 0;
					j = 0;

					if(!(strcmp(sin,"EXIT")))
						exit(EXIT_SUCCESS);

					if( write_line(fd, sin) <0 )
						perror("Failed to send a line ");
					else
						printf("%s%s%s","Message: \"",sin,"\" has been sent to the target.\n");
				}
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

	int flags =  fcntl(fd, F_GETFL, 0);
	if( fcntl(fd, F_SETFL, flags | FNDELAY) <0 )
		perror("failed to change tty atrb to no delay ");

	
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

