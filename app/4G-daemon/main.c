#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <net/if.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/times.h>
#include <dirent.h>

#define TTY_USB_NODE "/dev/ttyUSB1"
#define TEST_IP "8.8.8.8"
#define MAX_WAIT_TIME   1
#define MAX_NO_PACKETS  1
#define ICMP_HEADSIZE 8
#define PACKET_SIZE     1024

#define AT_RST_MODULE "AT+CFUN=1,1"
#define AT_CHK_MODULE "AT"
#define AT_DIS_ECHO "ATE0"

int net_state = 0;
int sockfd;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];
struct timeval tvsend,tvrecv;	
struct sockaddr_in dest_addr,recv_addr;
pid_t pid;

int tty_read_flag = 0;
char* tty_read_result = "";
char* tty_AT_node = NULL;

//functions
void *check_network(void *arg);
void dail();
bool net_is_ok();
void close_socket();
int pack(int pkt_no,char *sendpacket);
int send_packet(int pkt_no,char *sendpacket);
int recv_packet(int pkt_no,char *recvpacket);
int unpack(int cur_seq,char *buf,int len);
unsigned short cal_chksum(unsigned short *addr,int len);
void log_debug(const char *fmt, ...);
void reset_module();
void timeout(int signo);
void set_defroute();
int compatible_module();

//serial port operation
int open_tty(char *tty);
int config_tty(int fd);
int close_tty(int fd);
void send_AT_cmd(char *tty, char *AT_cmd);
void *read_tty(void *arg);

unsigned short cal_chksum(unsigned short *addr,int len)
{       
	int nleft=len;
	int sum=0;
	unsigned short *w=addr;
	unsigned short answer=0;
	while(nleft>1)		
	{       
		sum+=*w++;
		nleft-=2;
	}
	if( nleft==1)		
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer;
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return answer;
}


int unpack(int cur_seq,char *buf,int len)
{    
	int iphdrlen;
	struct ip *ip;
	struct icmp *icmp;
	ip=(struct ip *)buf;
	iphdrlen=ip->ip_hl<<2;		
	icmp=(struct icmp *)(buf+iphdrlen);
	len-=iphdrlen;
	if( len<8)
		return -1;       
	if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id==pid) && (icmp->icmp_seq==cur_seq))
		return 0;	
	else return -1;
}

int pack(int pkt_no,char*sendpacket)
{
    int i,packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp=(struct icmp*)sendpacket;
    icmp->icmp_type=ICMP_ECHO;   //设置类型为ICMP请求报文
    icmp->icmp_code=0;
    icmp->icmp_cksum=0;
    icmp->icmp_seq=pkt_no;
    icmp->icmp_id=pid;			//设置当前进程ID为ICMP标示符
    packsize=ICMP_HEADSIZE+sizeof(struct timeval);
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);
    icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize);
    return packsize;
}


int send_packet(int pkt_no,char *sendpacket)
{
    int packetsize;
    packetsize=pack(pkt_no,sendpacket);
    gettimeofday(&tvsend,NULL);
    if(sendto(sockfd,sendpacket,packetsize,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr) )<0) {
        log_debug("[NetStatus]  error : sendto error");
        return -1;
    }
    return 1;
}

int recv_packet(int pkt_no,char *recvpacket)
{
    int n,fromlen;
    fd_set rfds;
    struct timeval tv;
    signal(SIGALRM,timeout);
    tv.tv_sec = 5; //timeout 5s
    tv.tv_usec = 0;
    
    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);
    fromlen=sizeof(recv_addr);
    alarm(MAX_WAIT_TIME);
    while(1) {
        select(sockfd+1, &rfds, NULL, NULL, &tv);
        if (FD_ISSET(sockfd,&rfds)) {
            if( (n=recvfrom(sockfd,recvpacket,PACKET_SIZE,0,(struct sockaddr *)&recv_addr,&fromlen)) <0) {
                if(errno==EINTR)
                    return -1;
                perror("recvfrom error");
                return -2;
            }
        }else{
            log_debug("[NetStatus] recive packte error");
            return -3;
        }
        gettimeofday(&tvrecv,NULL);
        if(unpack(pkt_no,recvpacket,n)==-1)
            continue;
        return 1;
    }
}

/*
    note: Traffic 36B is consumed every 10 seconds, which is about 304K a day.
*/
bool net_is_ok()
{
    int iFlag;
    int ret;
    struct ifreq ifr;
    
    log_debug("ppp0 is exits!");
    
    dest_addr.sin_addr.s_addr = inet_addr(TEST_IP);
    
    //create icmp socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        log_debug("[NetStatus] socket create errir!");
        return false;
    }
    log_debug("[NetStatus] socket create success!");

    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, "ppp0", strlen("ppp0"));
    ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
    if( ret ) {
        log_debug("[NetStatus] band socket to ppp0 error ret = %d", ret);
        return false;
    }
    
    log_debug("[NetStatus] band socket to ppp0 success!");

    if(iFlag = fcntl(sockfd,F_GETFL,0)<0) {
        log_debug("[NetStatus]  error : fcntl(sockfd,F_GETFL,0)");
        close_socket();
        return false;
    }

    iFlag |= O_NONBLOCK;
    if(iFlag = fcntl(sockfd,F_SETFL,iFlag)<0) {
        log_debug("[NetStatus]  error : fcntl(sockfd,F_SETFL,iFlag )");
        close_socket();
        return false;
    }
    
    pid=getpid();
    
    for(int i=0; i<MAX_NO_PACKETS; i++) {

        if(send_packet(i,sendpacket)<0) {
            log_debug("[NetStatus]  error : send_packet");
            close_socket();
            return false;
        }
        log_debug("[NetStatus] send_packet success!");

        if(recv_packet(i,recvpacket)>0) {
            close_socket();
            log_debug("[NetStatus] recv_packet success!");
            return true;
        }
        
        log_debug("[NetStatus]recvpacket error!");

    }
    close_socket();
    return false;

}

void close_socket()
{
    close(sockfd);
    // log_debug("close_socket");
    sockfd = 0;
}

void timeout(int signo)
{
	log_debug("Request Timed Out");
}

void *check_network(void *arg)
{
    int ret;

    while (1) {
        sleep(20);
        //check ppp0 exits
        if (-1 == access("/sys/class/net/ppp0", F_OK)) {
            log_debug("ppp0 is not exits!");
            net_state = 0;
            continue;
        }
        //end check ppp0
        //check network is ok
        log_debug("check 4G network state...");
        if (!net_is_ok())
        {
            net_state = 0;
            log_debug("4G network is not available,reset the module");
            
            //reset module
            // system("sh /etc/ppp/peers/quectel-ppp-kill > /dev/null 2>&1");
            // reset_module();
            // sleep(10);
            
            continue;
        }else{
            log_debug("4G network is ok,and can ping to 8.8.8.8 address");
            net_state = 1;
        }
        //end check network
    };
}

void dail()
{
    // TODO, maybe need compatible with vendor id and pid
    if (! access(TTY_USB_NODE, F_OK)) {
        //file exit, then dial
        log_debug("%s node file exits", TTY_USB_NODE);
       
        // if 4G network is not ok，dail,default use ppp
        if ( net_state == 0 ) {
            system("sh /etc/ppp/peers/quectel-ppp-kill > /dev/null 2>&1");
            system("sh /etc/ppp/peers/quectel-pppd.sh > /dev/null 2>&1");
            net_state = 1;
        } else {
            log_debug("4G network is ok, not need dial");
        }
    } else {
        log_debug("%s node file not exits!", TTY_USB_NODE);
    }
}

void log_debug(const char *fmt, ...)
{
    char *debug = getenv("DEBUG_4G");
    
    if( NULL != debug && ! strcmp(debug, "1")){
        va_list args1;    
        va_list args2;   
        va_start(args1, fmt); 
        va_copy(args2, args1);
        char buf[1+vsnprintf(NULL, 0, fmt, args1)];   
        va_end(args1);    
        vsnprintf(buf, sizeof buf, fmt, args2);   
        va_end(args2); 

        printf("[4G-daemon]:%s\n", buf);
    }
}


int open_tty(char *tty){
    int fd;
    fd = open(tty, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY ); 
    
    if (fd > 0){
        if(fcntl(fd, F_SETFL, 0) < 0){
            log_debug("fcntl F_SETFL error!\n");
            close_tty(fd);
            return -1;
        }
    }
    return fd;
}
int convbaud(unsigned long int baudrate)
{
	switch (baudrate)
	{
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		default:
			return B115200;
	}
}

int config_tty(int fd)
{
	struct termios termios_old, termios_new;
	int baudrate, tmp;
	char databit, stopbit, parity;

	bzero(&termios_old, sizeof(termios_old));
	bzero(&termios_new, sizeof(termios_new));
	cfmakeraw(&termios_new);
	tcgetattr(fd, &termios_old);	// get the serial port
	// attributions
	// baudrates
	baudrate = convbaud(115200);
	cfsetispeed(&termios_new, baudrate);
	cfsetospeed(&termios_new, baudrate);
	termios_new.c_cflag |= (CLOCAL | CREAD);
	termios_new.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/*
	 * fctl = pportinfo-> fctl; switch(fctl){ case '0':{
	 * termios_new.c_cflag &= ~CRTSCTS; //no flow control }break; case 
	 * '1':{ termios_new.c_cflag |= CRTSCTS; //hardware flow control
	 * }break; case '2':{ termios_new.c_iflag |= IXON | IXOFF |IXANY;
	 * //software flow control }break; } 
	 */
	termios_new.c_cflag &= ~CSIZE;
	databit = 8;
	switch (databit)
	{
		case '5':
			termios_new.c_cflag |= CS5;
		case '6':
			termios_new.c_cflag |= CS6;
		case '7':
			termios_new.c_cflag |= CS7;
		default:
			termios_new.c_cflag |= CS8;
	}

	parity = 0;
	switch (parity)
	{
		case '0':
			{
				termios_new.c_cflag &= ~PARENB;	// no parity check
			}
			break;
		case '1':
			{
				termios_new.c_cflag |= PARENB;	// odd check
				termios_new.c_cflag |= PARODD;
			}
			break;
		case '2':
			{
				termios_new.c_cflag |= PARENB;	// even check
				termios_new.c_cflag = ~PARODD;
			}
			break;
		case '3':
			{
				termios_new.c_cflag = ~PARENB;	// space check
			}
	}

	stopbit = '1';
	if (stopbit == '2')
	{
		termios_new.c_cflag |= CSTOPB;	// 2 stop bits
	} else
	{
		termios_new.c_cflag &= ~CSTOPB;	// 1 stop bits
	}

	// other attributions default

	termios_new.c_oflag &= ~OPOST;
	termios_new.c_oflag &= ~(ONLCR | ICRNL);
	termios_new.c_iflag &= ~(ICRNL | INLCR);
	termios_new.c_iflag &= ~(IXON | IXOFF | IXANY);

	termios_new.c_cc[VMIN] = 0;
	termios_new.c_cc[VTIME] = 0;

	tcflush(fd, TCIFLUSH);
	tmp = tcsetattr(fd, TCSANOW, &termios_new);
	return (tmp);
}
/*
int config_tty(int fd){
    int ret;
    struct termios termios_old, termios_new;
    
    struct timeval tv;
    signal(SIGALRM,timeout);
    tv.tv_sec = 0; //timeout 5s
    tv.tv_usec = 50000;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    
    select(fd+1, &rfds, NULL, NULL, &tv);
    
    if(fd > 0){
        memset(&termios_old, 0, sizeof(struct termios));
        memset(&termios_new, 0, sizeof(struct termios));
    
        ret = tcgetattr(fd, &termios_old);//get serial port
        if(ret){
            log_debug("get serial port faild!");
            return ret;
        }
        
        ret = tcgetattr(fd, &termios_new);//get serial port
        if(ret){
            log_debug("get serial port faild!");
            return ret;
        }
        ret = cfsetispeed(&termios_new, B115200);
        ret |= cfsetospeed(&termios_new, B115200);
        if(ret){
            log_debug("set baudrate faild!");
            return ret;
        }

        termios_new.c_cflag |= CLOCAL | CREAD;//本地连接和接收使能

        termios_new.c_cflag &= ~CSIZE;//清空数据位
        termios_new.c_cflag |= CS8;//数据位为8位

        termios_new.c_cflag &= ~PARENB;//无奇偶校验

        termios_new.c_cflag &= ~CSTOPB;//一位停止位

        termios_new.c_cc[VTIME] = 1;//设置等待时间
        termios_new.c_cc[VMIN] = 1;
        
        ret = tcflush(fd, TCIFLUSH);
        if(ret){
            log_debug("failed to clear the cache: %s", strerror(errno));
            return ret;
        }
        
        ret = tcsetattr(fd, TCSANOW, &termios_new);
        if(ret){
            log_debug("failed to config serial port");
            return ret;
        }
        return 0;   
    }
    return -1;
}
*/
int close_tty(int fd){
    close(fd);
    return 0;
}

void *read_tty(void *fd)
{
    int cnt;
    int *a=1;
    char buf[100];
    tty_read_result="";
    while (tty_read_flag)
    {
        usleep(1000);
        //if(tty_read_flag){
            memset(buf,0,100);
            cnt = read((int)fd, buf, 100);
            //printf("cnt:%d",cnt);
        //}
        if(cnt >= 2){
            log_debug("uart read buffer is : %s\n", buf);
            tty_read_result = buf;
            pthread_exit(fd);
        }
    }
    pthread_exit(a);
}

void send_AT_cmd(char *tty , char *AT_cmd)
{
    int ret, fd;
    pthread_t uart_thread;
    void *lfd = NULL;
    fd = open_tty(tty);
    printf("tty:%s\n",tty);
    int fdcom=fd;
    if(fd == -1)
	{
		perror("Port Open Error:");
	}
	else if(fd > 0)
	{
		printf("Device opend,fd = %d\n",fd);
        config_tty(fd);
        // ret=write(fd, AT_DIS_ECHO, strlen(AT_DIS_ECHO));
        // if (ret == -1)
        //     fprintf(stderr, "fail write, error: %d, %s\n", errno, strerror(errno));

        tty_read_flag = 1;
        ret = pthread_create(&uart_thread, NULL, read_tty, fd);
        if(ret)
            log_debug("uart read thread create faild!");
        
        //pthread_detach(&uart_thread);
        
        ret=write(fd, AT_cmd, strlen(AT_cmd));
        printf("AT_cmd:%s\n",AT_cmd);
        if (ret == -1){
		    fprintf(stderr, "fail to write, error: %d, %s\n", errno, strerror(errno));
            // ret=write(fd, AT_CHK_MODULE, strlen(AT_CHK_MODULE));
            // if (ret == -1)
		    //     fprintf(stderr, "fail to write, error: %d, %s\n", errno, strerror(errno));
                
        }else if (ret)
            printf("write ret :%d\n", ret);
            sleep(1);
            tty_read_flag = 0;
            tcflush(fd, TCIFLUSH);
            tcflush(fd, TCOFLUSH);
            
            ret = pthread_join(uart_thread, &lfd);
            if(ret)
                log_debug("read thread join faild!");
            printf("return-lfd:%d\n", (int*)lfd);
            printf("fdcom:%d\n",fdcom);
            if((int*)lfd == fdcom)
            {
                log_debug("found AT command node: %s", tty);
                tty_AT_node=malloc(strlen(tty)+1);
                memcpy(tty_AT_node, tty, strlen(tty)+1);
                printf("tty_AT_node:%s\n",tty_AT_node);
                // ret=write(fd, AT_DIS_ECHO, strlen(AT_DIS_ECHO));
                // if (ret == -1)
		        //     fprintf(stderr, "fail write, error: %d, %s\n", errno, strerror(errno));
            }
            close_tty(fd);

    }
        else{
            tty_read_result = "";
        }  
}   

int compatible_module()
{
    if (! access(TTY_USB_NODE, F_OK)) {
        DIR *dir;			
        if((dir = opendir("/dev/"))==0){       
            log_debug("dir /dev  open failed!");
            return -1;
        }
        struct dirent *stdir;
        while ((stdir = readdir(dir)) != NULL){
            char node_path[50] = "/dev/";
            if(strstr(stdir->d_name, "ttyUSB")){
                
                strcat(node_path, stdir->d_name);
                log_debug("node_path %s", node_path);
                send_AT_cmd(node_path, AT_CHK_MODULE);
                
                sleep(1);
                if(tty_read_result != ""){
                    //log_debug("found AT command node: %s", node_path);
                    //memcpy(tty_AT_node, node_path, strlen(node_path)+1);
                    break;
                }
            }
        }
        closedir(dir);
    }
}

void reset_module()
{
    printf("reset module!\n");
    // char *buf=(char *)malloc(50);
    // sprintf(buf,"echo %s > %s\n",AT_RST_MODULE,tty_AT_node);
    // printf("buf:%s\n",buf);
    // system(buf);
    // printf("system_success\n");

    // free(buf);

    send_AT_cmd(tty_AT_node, AT_RST_MODULE);

}


int main()
{
    int ret;
    pthread_t net_thread;

    compatible_module();

    ret = pthread_create(&net_thread, NULL, check_network, NULL);
    if (ret != 0) {
        log_debug("pthread_create error: error_code = %d", strerror(errno));
    }

    while (1) {
        dail();
        sleep(5); //5s 
    }

    return 0;
}

