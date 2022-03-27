CC = g++
CFLAGS	+=-Wall -g

LIB = libminidb.a
LLIB = minidb

SRC = test.cpp
TARGET = test

SUB_DIR = src

all:$(LIB)
	$(CC) $(SRC) -L. -l$(LLIB) -o $(TARGET) $(CFLAGS)

$(LIB):
	make -C $(SUB_DIR)

help:
	@echo "make help 打印帮助；make build 编译静态库；make test 压力测试；make clean 删除编译结果"

clean:
	-$(RM) $(TARGET) *.a $(SUB_DIR)/*.o

.PHONY:clean all help
