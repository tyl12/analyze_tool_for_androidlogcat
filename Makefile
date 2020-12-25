CFLAGS += -Wall -I./libs
LDFLAGS += -L./libs -lutils
all:
	g++ -std=c++20 $(CFLAGS) process.cpp -o process $(LDFLAGS)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./libs ./process -s "callback status : 0|稳定点达到，开始结算|上传中...|上传 成功" -i "|" -d -f "res/2020-10-21-11"
