/* 
 * udp epoll simple: dns query and response multiply
 */

#include	<stdio.h>
#include	<sys/epoll.h>
#include	<sys/time.h>
#include	<signal.h>
#include	"unp.h"

#define SERV_NR 32


struct server_node{
	int fd;
	char *server_ip;
	struct timeval t;
};

void epoll_tm_handler(int signo){
//	printf("sig\n");
}


int
main(int argc, char **argv)
{
	int	sockfd;
	struct server_node	sn[SERV_NR], *sn_tmp;
	struct timeval at;
	struct itimerval delay;
	unsigned long udelay;
	int timer_ret;
	struct sigaction tm_action;
	
	struct sockaddr_in	servaddr;
	int i=0;

	/* dns query: www.google.com */
	unsigned char msg[32] = {0x05,0xf7,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x03, \
			0x77,0x77,0x77,0x06,0x67,0x6f,0x6f,0x67,0x6c,0x65,0x03,0x63,0x6f,0x6d,0x00,0x00,\
			0x01,0x00,0x01};

	int		event_nr;
	char		buf[BUFFSIZE];
	int		n;
	int 		epfd;
	struct 		epoll_event sock_event[SERV_NR];
	struct 		epoll_event callback_events[SERV_NR];
	int		serv_nr;

	if (argc < 2)
		err_quit("usage: udpcli <DNS_SERVER1> <DNS_SERVER2> ...");
	
	serv_nr = argc-1;

	delay.it_value.tv_sec = 5;
	delay.it_value.tv_usec = 0;
	delay.it_interval.tv_sec = 0;
	delay.it_interval.tv_usec = 0;

	tm_action.sa_handler = &epoll_tm_handler;
	tm_action.sa_flags = SA_SIGINFO|SA_RESTART|SA_RESETHAND;

	epfd = epoll_create(SERV_NR);
	if (epfd < 0)
		err_quit("epoll_create");

	for (;i<serv_nr;i++){
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(53);
		Inet_pton(AF_INET, argv[i+1], &servaddr.sin_addr);
		sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
		sn[i].fd = sockfd;
		sn[i].server_ip = argv[i+1];
		sock_event[i].data.ptr = &sn[i];
		sock_event[i].events = EPOLLIN;
		if(epoll_ctl (epfd, EPOLL_CTL_ADD, sockfd, &sock_event[i]))
			err_quit("epoll_ctl");
		gettimeofday(&sn[i].t,NULL);
		Sendto(sockfd, msg, 32, 0, (SA *)&servaddr, sizeof(struct sockaddr_in));
	}
	
	sigaction(SIGALRM, &tm_action, NULL);
	timer_ret = setitimer(ITIMER_REAL, &delay, NULL);
	if(timer_ret){
		err_quit("setitimer");
	}

	while (serv_nr>0) {
		event_nr = epoll_wait(epfd, (struct epoll_event *)&callback_events, SERV_NR, 1000);
		if (event_nr < 0){
			err_quit("timeout");
		}else if(event_nr == 0){
			printf(".");
			fflush(stdout);
		}
		for (i=0; i<event_nr; i++){
			sn_tmp = (struct server_node *)(callback_events[i].data.ptr);
			gettimeofday(&at, NULL);
			udelay =  (at.tv_sec-(sn_tmp->t).tv_sec)*1000000+(at.tv_usec-(sn_tmp->t).tv_usec);
			n = Read(sn_tmp->fd, buf, BUFFSIZE);
			if (n == 0) {
				err_quit("Read nothing");
			}
			if(epoll_ctl(epfd, EPOLL_CTL_DEL, sn_tmp->fd, NULL)){
				err_quit("epoll_ctl del");
			}
			memset(buf, 0, BUFFSIZE);
			close(sn_tmp->fd);
			printf("response from server: %s\t%lu\n",sn_tmp->server_ip,udelay/1000);
		}
		serv_nr -= event_nr;
	}
	close(epfd);
	printf("\n");
	exit(0);
}
