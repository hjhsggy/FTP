target:
	#生成动态库
	make -C ./dynamic/01condition/
	make -C ./dynamic/02common/
	make -C ./dynamic/03Login_ftp/
	make -C ./dynamic/04database/
	make -C ./dynamic/05log/
	#将动态库文件拷贝到lib目录
	cp ./dynamic/01condition/libdynamic_condition.so ./lib/
	cp ./dynamic/02common/libdynamic_common.so ./lib/
	cp ./dynamic/03Login_ftp/libdynamic_login_ftp.so ./lib/
	cp ./dynamic/04database/libdynamic_mysql_c.so ./lib/
	cp ./dynamic/05log/libdynamic_log.so ./lib/
	#为lib目录下的动态库文件创建软链接
	sudo ln -s /home/sunxiwang/03LinuxStudy/workspace/100project/02FTP/lib/libdynamic_condition.so  /usr/lib/libdynamic_condition.so
	sudo ln -s /home/sunxiwang/03LinuxStudy/workspace/100project/02FTP/lib/libdynamic_common.so  /usr/lib/libdynamic_common.so
	sudo ln -s /home/sunxiwang/03LinuxStudy/workspace/100project/02FTP/lib/libdynamic_login_ftp.so  /usr/lib/libdynamic_login_ftp.so
	sudo ln -s /home/sunxiwang/03LinuxStudy/workspace/100project/02FTP/lib/libdynamic_mysql_c.so  /usr/lib/libdynamic_mysql_c.so
	sudo ln -s /home/sunxiwang/03LinuxStudy/workspace/100project/02FTP/lib/libdynamic_log.so  /usr/lib/libdynamic_log.so
	#生成可执行程序
	make -C ./src/01server/
	make -C ./src/02client/

install:
	#将可执行文件拷贝到/bin目录下
	cp ./src/01server/server ./bin/serverExe/
	cp ./src/02client/client ./bin/clientExe/
uninstall:
	#卸载
	rm -rf ./bin/serverExe/server
	rm -rf ./bin/clientExe/client

.PHONY:clean
clean:
	make clean -C ./src/01server/
	make clean -C ./src/02client/
	make clean -C ./dynamic/01condition/
	make clean -C ./dynamic/02common/
	make clean -C ./dynamic/03Login_ftp/
	make clean -C ./dynamic/04database/
	make clean -C ./dynamic/05log/
	rm -rf ./lib/*
	sudo rm -rf /usr/lib/libdynamic_condition.so  /usr/lib/libdynamic_common.so
	sudo rm -rf /usr/lib/libdynamic_login_ftp.so  /usr/lib/libdynamic_mysql_c.so
	sudo rm -rf /usr/lib/libdynamic_log.so  

