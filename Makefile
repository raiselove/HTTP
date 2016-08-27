PWD=$(shell pwd)
CGI_PATH=$(PWD)/cgi_bin 
SER_BIN=http
CLI_BIN=client
SER_SRC=http.c
CLI_SRC=client.c
INCLUDE=.
CC=gcc
FLAGS=-o 
LDFLAGS=-lpthread#-static
LIB=

.PHONY:all
all:$(SER_BIN) $(CLI_BIN) cgi
$(SER_BIN):$(SER_SRC)
		$(CC) $(FLAGS) $@ $^ $(LDFLAGS) -D_DEBUG_
$(CLI_BIN):$(CLI_SRC)
		$(CC) $(FLAGS) $@ $^ $(LDFLAGS)
.PHONY:cgi
cgi:
	for name in `echo $(CGI_PATH)`;\
	do\
		cd $$name;\
		make;\
		cd -;\
	done
.PHONY:output
output:
	mkdir -p output/htdoc/cgi
	cp http output
	cp client output
	cp -rf config output
	cp -rf log output
	cp start.sh output
	cp -rf htdoc/* output/htdoc
	for name in `echo $(CGI_PATH)`;\
		do\
			cd $$name;\
			make output;\
			cd -;\
		done

.PHONY:clean
clean:
	rm -rf $(SER_BIN) $(CLI_BIN) output
	for name in `echo $(CGI_PATH)`;\
	do\
		cd $$name;\
		make clean;\
		cd -;\
		done
	
