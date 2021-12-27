##
# Lars
#
# @Makefile
# @version 0.1
TARGET=lib/liballoc.a
CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated

SRC=./src

INC  =-I ./include
OBJS = $(addsuffix .o, $(basename $(wildcard $(SRC)/*.cpp)))
LIB  =-L./lib -lalloc

all: $(TARGET)
	$(CXX) -o main $(CFLAGS) main.cpp $(INC) $(LIB)

$(TARGET): $(OBJS)
	mkdir -p lib
	ar cqs $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC)

.PHONY: clean

clean:
	-rm -f src/*.o $(TARGET) main

# end
