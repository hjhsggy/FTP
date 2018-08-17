SRC=$(wildcard ./*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
LIB=./libdynamic_log.so 

$(LIB):$(OBJ)
	gcc $^ -o $@ --share -lzlog

./%.o:./%.c
	gcc -c $< -o $@ -I../../inc/ -fPIC

clean:
	rm $(OBJ) $(LIB)
