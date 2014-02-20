#include	"unp.h"
#include	"sys/epoll.h"

void
str_cli(FILE *fp, int sockfd)
{
	int		event_nr, stdineof;
	char		buf[MAXLINE];
	int		i, n;

	int epfd;
	struct epoll_event sock_event;
	struct epoll_event fp_event;
	struct epoll_event events;

	sock_event.data.fd = sockfd;
	sock_event.events = EPOLLIN;
	fp_event.data.fd = fileno(fp);
	fp_event.events = EPOLLIN;

	epfd = epoll_create(100);
	if (epfd < 0)
		err_quit("epoll_create");

	if (epoll_ctl (epfd, EPOLL_CTL_ADD, sockfd, &sock_event))
		err_quit("epoll_ctl");

	if (epoll_ctl (epfd, EPOLL_CTL_ADD, fileno(fp), &fp_event))
		err_quit("epoll_ctl");

	for ( ; ; ) {
		event_nr = epoll_wait(epfd, &events, 100, -1);
		if (event_nr < 0){
			err_quit("epoll_wait");
		}

		for (i=0; i<event_nr; i++){
			if (sockfd == events.data.fd) {	/* socket is readable */
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
					if (stdineof == 1)
						return;		/* normal termination */
					else
						err_quit("str_cli: server terminated prematurely");
				}

				Write(fileno(stdout), buf, n);
			}

			if (fileno(fp) == events.data.fd) {  /* input is readable */
				if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) {
					stdineof = 1;
					Shutdown(sockfd, SHUT_WR);	/* send FIN */
					epoll_ctl (epfd, EPOLL_CTL_DEL, fileno(fp), &fp_event);
					continue;
				}

				Writen(sockfd, buf, n);
			}
		}
	}
	close(epfd);
}
