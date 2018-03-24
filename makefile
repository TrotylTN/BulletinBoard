CC = g++
CFLAGS = -D_REENTRANT -std=c++11
# Disabled warnings, remember to clear it before debugging
LDFLAGS = -lpthread

all: BoardServer BoardClient


BoardServer: boardServer.cpp comm.h comm.cpp server_comm.h server_comm.cpp server_seq.cpp server_quorum.cpp server_rnw.cpp
	${CC} boardServer.cpp comm.cpp server_comm.cpp server_seq.cpp server_quorum.cpp server_rnw.cpp -o BoardServer ${LDFLAGS} ${CFLAGS}

BoardClient: boardClient.cpp comm.h comm.cpp client_comm.h client_comm.cpp
	${CC} boardClient.cpp comm.cpp client_comm.cpp -o BoardClient ${LDFLAGS} ${CFLAGS}

test: test.cpp
	g++ test.cpp comm.cpp -o test -std=c++11

clean:
	rm -rf BoardServer BoardClient
