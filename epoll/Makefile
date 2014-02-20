include ../Make.defines

PROGS =	tcpcli_epoll

all:	${PROGS}

tcpcli_epoll:	tcpcli01.o  strclipoll01.o
		${CC} ${CFLAGS} -o $@ tcpcli01.o strclipoll01.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
