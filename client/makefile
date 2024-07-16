#指定可执行文件名字，根据实际情况写
TARGET:=ftpclient

#指定编译器的名字，根据实际情况写
CC:=gcc

#指定.c文件的路径，根据实际情况写，是.c文件相对makefile的路径
SRCPATH:=.
#根据路径得到 .c文件列表
SRCNAME:=$(wildcard $(SRCPATH)/*.c)

#../src下也有 .c文件，所以也要获取
SRCPATH:=../src
SRCNAME+=$(wildcard $(SRCPATH)/*.c)

#根据.c文件列表得到 .o文件列表
OBJENAME:=$(patsubst %.c,%.o,$(SRCNAME))

#指定头文件的路径，根据实际情况写，是.h文件相对makefile的路径
INC:=-I ../inc

#指定库文件的路径及名字，根据实际情况写
#LIB:=-L ./code/lib -l list
#LIB:=-pthread
#如果库文件有多个路径，根据实际情况追加
#LIB+=-L 库文件的路径 -l 库文件的名字

$(TARGET):$(OBJENAME)
	$(CC)  $(OBJENAME) -o $(TARGET) $(LIB) 

#打印输出验证是否正确
#	echo $(SRCNAME)
#	echo $(OBJENAME)

%.o : %.c
	$(CC)  -c $^ -o $@ $(INC)

#删除所有.o文件及最终的可执行文件，一般在重新编译之前使用
clear:
	rm $(TARGET)
	rm *.o
	rm $(SRCPATH)/*.o
