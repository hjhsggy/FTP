SRC=$(wildcard ./*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
LIB=./libdynamic_condition.so

$(LIB):$(OBJ)
	gcc $^ -o $@ --share

./%.o:./%.c
	gcc -c $< -o $@ -fPIC -I../../inc

clean:
	rm $(OBJ) $(LIB)
