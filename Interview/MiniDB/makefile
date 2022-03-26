CFLAGS	+=-Wall -g -lstdc++ -lpthread
target = minidb
src = ./db_file.cpp ./db.cpp

build:$(src)
	gcc $(src) -I./ -o $(target) $(CFLAGS)

clean:
	-rm $(target)

.PHONY:clean build
