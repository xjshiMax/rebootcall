CC=gcc
CFLAGS=  -g -Wall -Wno-unused-variable -pthread   -lstdc++ -lrt -lm
INCLUDE=-I./ -I./database -I./tx_dep/mysqlcnn/include/jdbc/ -I./base/include/
LIBS= -L./base/glog/linux -lglog \
	-L./base/inifile/lib -linifile \
	-L./tx_dep/mysqlcnn/lib64 -lcrypto -lmysqlcppconn8 -lmysqlcppconn -lssl -lcurl
SRC= main.cpp TXTCPServer.cpp process_event.cpp \
	./base/include/xthreadPool.cpp ./base/include/xthreadbase.cpp ./base/include/xtimeheap.cpp ./base/include/xEventDemultiplexer.cpp\
	./base/include/xReactor.cpp \
	./database/dbPool.cpp ./database/config/inirw.cpp \
	./common/speech/common.c  ./common/speech/token.cpp  ./common/speech/ttscurl.c \
	./common/codeHelper.cpp ./common/structdef.cpp ./common/DBOperator.cpp ./esl/esl.c \
	./esl/esl_json.c ./esl/esl_threadmutex.c \
	./esl/esl_buffer.c ./esl/esl_event.c ./esl/esl_config.c
	
target:gf_txcall
gf_txcall:$(SRC)
	$(CC) -o $@ $^ $(INCLUDE) $(CFLAGS) $(LIBS) 

clean:
	rm -f txcall *.o *.obj
