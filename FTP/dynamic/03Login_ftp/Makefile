SRC=$(wildcard ./*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
LIB=./libdynamic_login_ftp.so 

$(LIB):$(OBJ)
	gcc $^ -o $@ --share -lmysqlclient

./%.o:./%.c
	gcc -c $< -o $@ -fPIC -I../../inc

clean:
	rm $(OBJ) $(LIB)
