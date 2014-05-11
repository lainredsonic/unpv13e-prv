include ../Make.defines

PROGS =	tcpcli_epoll udpcli01

all:	${PROGS}

tcpcli_epoll:	tcpcli01.o  strclipoll01.o
		${CC} ${CFLAGS} -o $@ tcpcli01.o strclipoll01.o ${LIBS}

udpcli01: udpcli01.o
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
