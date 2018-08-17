SRC=$(wildcard ./*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
LIB=./libdynamic_common.so

$(LIB):$(OBJ)
	gcc $^ -o $@ --share 

./%.o:./%.c
	gcc -c $< -o $@ -fPIC -I../../inc -lconfig

clean:
	rm $(OBJ) $(LIB)
