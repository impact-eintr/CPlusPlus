CC = g++
CFLAGS	+=-Wall -g -std=c++11 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
target = ../libminidb.a
src = db_file.cpp db.cpp timer/*.cpp
obj = *.o

build:$(target)

$(target):$(obj)
	ar crv $@ $^

$(obj):$(src)
	$(CC) $(src) -I./ -c $(CFLAGS)

clean:
	-rm $(obj) $(target)

.PHONY:clean build
