all:
	g++ -std=c++20 -Wall process.cpp -o process -L./ -lutils
	./process -s "callback status : 0|稳定点达到，开始结算|上传中...|上传 成功" -i "|" -d -f "res/2020-10-21-11"
