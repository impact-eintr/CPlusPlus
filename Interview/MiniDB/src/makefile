CC = g++
CFLAGS	+=-Wall -g
target = ../libminidb.a
src = db_file.cpp db.cpp
obj = $(patsubst %.cpp,%.o,$(src))

build:$(target)

$(target):$(obj)
	ar crv $@ $^

$(obj):$(src)
	$(CC) $(src) -I./ -c $(CFLAGS)

clean:
	-rm $(obj) $(target)

.PHONY:clean build
