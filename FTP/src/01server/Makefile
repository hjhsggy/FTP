SRC=$(wildcard ./*.c)
LIB = -ldynamic_condition -ldynamic_common -ldynamic_login_ftp -ldynamic_mysql_c -ldynamic_log

server:$(SRC)
	gcc $^ -o $@ -g -pthread  -L../../lib $(LIB) -I../../inc -lzlog -lconfig
.PHONY:clean
clean:
	rm server

