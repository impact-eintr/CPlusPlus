CFLAGS	+=-Wall -g -lstdc++ -lpthread
lib = minidb
target = libminidb.a
ttarget = test
src = ./db_file.cpp ./db.cpp
tsrc = ./test.cpp
obj = $(patsubst %.cpp,%.o,$(src))

help:
	@echo "make help 打印帮助；make build 编译静态库；make test 压力测试；make clean 删除编译结果"

build:$(target)

$(target):$(obj)
	ar crv $@ $^

$(obj):$(src)
	gcc $(src) -I./ -c $(CFLAGS)

test:$(target)
	gcc $(tsrc) -L. -l$(lib) -o $(ttarget) $(CFLAGS)

clean:
	-rm $(obj) $(target) $(ttarget)

.PHONY:clean build test help
