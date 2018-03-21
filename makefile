CC = g++
CFLAGS = -D_REENTRANT -std=c++11 -w
# Disabled warnings, remember to clear it before debugging
LDFLAGS = -lpthread

all: BoardServer BoardClient


BoardServer: boardServer.cpp comm.h
	${CC} boardServer.cpp -o BoardServer ${LDFLAGS} ${CFLAGS}

BoardClient: boardClient.cpp comm.h
	${CC} boardClient.cpp -o BoardClient ${LDFLAGS} ${CFLAGS}

clean:
	rm -rf BoardServer BoardClient