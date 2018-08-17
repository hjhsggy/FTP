SRC=$(wildcard ./*.c)
LIB = -ldynamic_condition -ldynamic_common -lconfig

client:$(SRC)
	gcc $^ -o $@ -g -pthread  -L../../lib $(LIB) -I../../inc
.PHONY:clean
clean:
	rm client

