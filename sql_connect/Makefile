WORK_PATH=$(shell pwd)
BIN=mysql_connect
SRC=mysql_connect.cpp
CC=g++
INCLUDE=-I${WORK_PATH}/mysql_lib/include 
LIB_PATH=-L${WORK_PATH}/mysql_lib/lib

LIB_NAME=-lmysqlclient

$(BIN):$(SRC)
	$(cc) -o $@ $^ ${INCLUDE} ${LIB_PATH} ${LIB_NAME} -D_DEBUG_
	#链接mysqlclient库
.PHONY:clean
clean:
	rm -rf $(BIN)
