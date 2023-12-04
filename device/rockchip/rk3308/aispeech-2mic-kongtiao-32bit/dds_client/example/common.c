#include "common.h"

int writeport(int *fd, unsigned char *chars,int len) {

        //printf("*fd = %d\n",getbaud(*fd));
        int n = write(*fd, chars,len);
        if (n < 0) {
                fputs("write failed!\n", stderr);
		        printf("\n written %d bytes to port fail:%d\n",len, n);
                return n;
        }
        printf("\n written %d bytes to port \n",n);
        return n;
}

int readport(int *fd, unsigned char *result) {
        int iIn=0;
	iIn = read(*fd, result, bufsize);
	if (iIn < 0) {
        if (errno == EAGAIN) {
                printf("SERIAL EAGAIN ERROR\n");
                return 0;
        } else {
              printf("SERIAL read error %d %s\n", errno, strerror(errno));
                return 0;
        }
    }
	//printf("\n read %d bytes from port",iIn); 

	/* If we got an expected response trailer, break. */
	if(iIn ==1 && result[0] == 0x00 ) {
		//printf("\n iIn ==1 and 0x00 character \n"); 
		return 0;
	}

        return iIn;
}


int getbaud(int fd) {
        struct termios termAttr;
        int Speed = -1;
        speed_t baudRate;
        tcgetattr(fd, &termAttr);
        /* Get the input speed.                              */
        baudRate = cfgetispeed(&termAttr);
	switch (baudRate) {
		case B300:    Speed = 300; break;
		case B600:    Speed = 600; break;
		case B1200:   Speed = 1200; break;
		case B1800:   Speed = 1800; break;
		case B2400:   Speed = 2400; break;
		case B4800:   Speed = 4800; break;
		case B9600:   Speed = 9600; break;
		case B19200:  Speed = 19200; break;
		case B38400:  Speed = 38400; break;
		case B57600:  Speed = 57600; break;
		case B115200: Speed = 115200; break;
		case B230400: Speed = 230400; break;
		case B460800: Speed = 460800; break;
		case B500000: Speed = 500000; break;
		case B576000: Speed = 576000; break;
		case B921600: Speed = 921600; break;
		case B1000000:Speed = 1000000; break;
		case B1152000:Speed = 1152000; break;
		case B1500000:Speed = 1500000; break;
		case B2000000:Speed = 2000000; break;
		case B2500000:Speed = 2500000; break;
		case B3000000:Speed = 3000000; break;
		case B3500000:Speed = 3500000; break;
		case B4000000:Speed = 4000000; break;
    }
    return Speed;
}

int initport(int fd,long baudrate,int flow_ctrl) 
{
	struct termios options;
        memset(&options, 0 , sizeof(options));
        if(ERROR == tcgetattr(fd, &options)) {
                perror("tcgetattr: ");
                return ERROR;
        }
	printf("buadrate:%d\n", baudrate);
	options.c_cflag &= ~CBAUD;

	switch (baudrate) {
		case 300:	cfsetispeed(&options,B300); 
				cfsetospeed(&options,B300);
				options.c_cflag |= B300;
				break;
		case 600:	cfsetispeed(&options,B600);
				cfsetospeed(&options,B600);
				options.c_cflag |= B600;
				break;
		case 1200:	cfsetispeed(&options,B1200);
				cfsetospeed(&options,B1200);
				options.c_cflag |= B1200;
				break;
		case 1800:	cfsetispeed(&options,B1800);
				cfsetospeed(&options,B1800);
				options.c_cflag |= B1800;
				break;
		case 2400:	cfsetispeed(&options,B2400);
				cfsetospeed(&options,B2400);
				options.c_cflag |= B2400;
				break;
		case 4800:	cfsetispeed(&options,B4800);
				cfsetospeed(&options,B4800);
				options.c_cflag |= B4800;
				break;
		case 9600:  
		default:	cfsetispeed(&options,B9600);
				cfsetospeed(&options,B9600);
				options.c_cflag |= B9600;
				break;
		case 19200:	cfsetispeed(&options,B19200);
				cfsetospeed(&options,B19200);
				options.c_cflag |= B19200;
				break;
		case 38400:	cfsetispeed(&options,B38400);
				cfsetospeed(&options,B38400);
				options.c_cflag |= B38400;
				break;
                case 57600:     cfsetispeed(&options,B57600);
                                cfsetospeed(&options,B57600);
                                options.c_cflag |= B57600 | CBAUDEX;
                                break;	
		case 115200:	cfsetispeed(&options,B115200);
				cfsetospeed(&options,B115200);
				options.c_cflag |= B115200 | CBAUDEX;
				break;
		case 230400:	cfsetispeed(&options,B230400);
				cfsetospeed(&options,B230400);
				options.c_cflag |= B230400 | CBAUDEX;
				break;
		case 460800:	cfsetispeed(&options,B460800);
				cfsetospeed(&options,B460800);
				options.c_cflag |= B460800 | CBAUDEX;
				break;
		case 500000:	cfsetispeed(&options,B500000);
				cfsetospeed(&options,B500000);
				options.c_cflag |= B500000 | CBAUDEX;
				break;
		case 576000:	cfsetispeed(&options,B576000);
				cfsetospeed(&options,B576000);
				options.c_cflag |= B576000 | CBAUDEX;
				break;
		case 921600:	cfsetispeed(&options,B921600);
				cfsetospeed(&options,B921600);
				options.c_cflag |= B921600 | CBAUDEX;
				break;
		case 1000000:	cfsetispeed(&options,B1000000);
				cfsetospeed(&options,B1000000);
				options.c_cflag |= B1000000 | CBAUDEX;
				break;
		case 1152000:	cfsetispeed(&options,B1152000);
				cfsetospeed(&options,B1152000);
				options.c_cflag |= B1152000 | CBAUDEX;
				break;
		case 1500000:	cfsetispeed(&options,B1500000);
				cfsetospeed(&options,B1500000);
				options.c_cflag |= B1500000 | CBAUDEX;
				break;
		case 2000000:	cfsetispeed(&options,B2000000);
				cfsetospeed(&options,B2000000);
				options.c_cflag |= B2000000 | CBAUDEX;
				break;
		case 2500000:	cfsetispeed(&options,B2500000);
				cfsetospeed(&options,B2500000);
				options.c_cflag |= B2500000 | CBAUDEX;
				break;
		case 3000000:	cfsetispeed(&options,B3000000);
				cfsetospeed(&options,B3000000);
				options.c_cflag |= B3000000 | CBAUDEX;
				break;
		case 3500000:	cfsetispeed(&options,B3500000);
				cfsetospeed(&options,B3500000);
				options.c_cflag |= B3500000 | CBAUDEX;
				break;
		case 4000000:	cfsetispeed(&options,B4000000);
				cfsetospeed(&options,B4000000);
				options.c_cflag |= B4000000 | CBAUDEX;
				break;
	}
        
	/* Enable or Disable Hardware flow control. */
    if(!flow_ctrl) {
		options.c_cflag &= ~CRTSCTS;
		printf("\n Flow control disabled \n");
	}
	else {
		options.c_cflag |= CRTSCTS;
		printf("\n Flow control enabled \n");
	}

        /* No Parity, 8N1. */
        options.c_cflag &= ~PARENB; // No parity bit
        options.c_cflag &= ~CSTOPB; // 1 stop bit
        options.c_cflag &= ~CSIZE;  // character size 8
        options.c_cflag |=  CS8 ;

        /* Hardware Control Options - Set local mode and Enable receiver to receive characters */
        options.c_cflag     |= (CLOCAL | CREAD );
	
        /* Terminal Control options */
        /* - Disable signals. Disable canonical input processing. Disable echo. */
        options.c_lflag     &= ~(ICANON | IEXTEN | ECHO | ISIG); /* Line options - Raw input */
        
		options.c_iflag     &= ~(ICRNL | INPCK | ISTRIP | BRKINT | IXON | IXOFF | IXANY);
        /* Output processing - Disable post processing of output. */
        options.c_oflag     &= ~OPOST;      /* Output options - Raw output */
        /* Control Characters - Min. no. of characters */
        options.c_cc[VMIN]  = 0;
        /* Character/Packet timeouts. */
        options.c_cc[VTIME] = 3;

	if(ERROR == tcflush(fd,TCOFLUSH) ) {
                  printf("tcflush failed");
                  return ERROR;
        }

	if(ERROR == tcsetattr(fd, TCSANOW, &options))  {
                printf("Error: Couldn't configure Serial port " );
                return ERROR;
        }
        
/*
	printf("\n\n");
	system("stty -F /dev/ttyS0 -a");
	printf("\n\n"); 
*/
	return SUCCESS;
}

int serial_openport(long int baudrate) {
    /*int*/ ut.fd = open("/dev/ttyS0", O_RDWR);
    if(ut.fd < 0) {
        printf("open port failed\n");
        return -1;
    }
    fcntl(ut.fd, F_SETFL, 0);
    printf("baudrate:%ld\n",baudrate);
    printf("\n Existing port baud=%d", getbaud(ut.fd));
    tcgetattr(ut.fd, &oldtio);
    initport(ut.fd, baudrate, 0);
    printf("get baudrate :%d \n",getbaud(ut.fd));
    return ut.fd; 
}

int serial_send_data(int fd,const unsigned char* data) {
    int i,ret;
    printf("\n");
    for(int i = 0;i<strlen(data);i++)
        printf("%d,",data[i]);
    printf("\n");
    fcntl(fd, F_SETFL, 0);
    if(strlen(data) < 8) {
        ret = writeport(&fd,data,strlen(data));
    } else {
        ret = writeport(&fd,data,8);
    }
    //ioctl(fd, TCSBRK, 1);
  /*  if(tcdrain(fd) < 0)
        printf("\n tcdrain failure \n");
    sleep(3);
    for(i=0;i<5;i++)
        tcsendbreak(fd,5);*/ 
    return ret;
}


void signalHandler()
{
	printf("\n Closing device %s\n",UART_DEV_NAME);
	gettimeofday(&ut.end_time,NULL);
	timersub(&ut.end_time,&ut.start_time,&ut.diff_time);
	
	if(read_flag && tx_rx == 'r') {
	        ut.diff_time.tv_sec -= 5;
	        printf("\n Time taken %08ld sec, %08ld usec\n\n ",ut.diff_time.tv_sec,ut.diff_time.tv_usec);
	}
	if(tx_rx == 's') 
	        printf("\n Time taken %08ld sec, %08ld usec\n\n ",ut.diff_time.tv_sec,ut.diff_time.tv_usec);

	close_port();
}

void close_port()
{
        tcsetattr(ut.fd, TCSANOW, &oldtio);
	if(tx_rx == 'r')
	        close(fd2);
	else
        	close(fd1);
        close(ut.fd);
        exit(1);
}

void display_intro()
{
	printf("\n Use the following format to run the HS-UART TEST PROGRAM\n");
	printf(" ts_uart v1.1\n");
	printf(" For sending data: \n ./ts_uart <tx_rx(s/r)> <file_name> <baudrate> <flow_control(0/1)> <max_delay(0-100)> <random_size(0/1)>\n");
	printf(" tx_rx : send data from file (s) or receive data (r) to put in file\n");
	printf(" file_name : file name to send data from or place data in\n");
	printf(" baudrate : baud rate used for TX/RX\n");
	printf(" flow_control : enables (1) or disables (0) Hardware flow control using RTS/CTS lines\n");
	printf(" max_delay : defines delay in seconds between each data burst when sending. Choose 0 for continuous stream.\n");
	printf(" random_size : enables (1) or disables (0) random size data bursts when sending. Choose 0 for max size.\n");
	printf(" max_delay and random_size are useful for sleep/wakeup over UART testing. ONLY meaningful when sending data\n");
	
	printf(" Examples:\n");
	printf(" Sending data (no delays)\n");
	printf(" ts_uart s init.rc 1500000 0 0 0 /dev/ttyS0\n");
	printf(" loop back mode:\n"); 
	printf(" ts_uart m init.rc 1500000 0 0 0 /dev/ttyS0\n");
	printf(" receive, data must be 0x55\n"); 
	printf(" ts_uart r init.rc 1500000 0 0 0 /dev/ttyS0\n");
	
}

/* return a random value between 0 and max */ 
int get_rand_value(int max){
	int value = (int) ((float) rand() / (float) RAND_MAX * max); /* map random size within 0 - max range*/
	return value;
}

