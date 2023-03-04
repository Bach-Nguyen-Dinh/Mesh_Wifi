CC=g++
CFLAGS=-c -Wall
SOURCES = $(wildcard *.cpp)
EXECS = $(SOURCES:%.cpp=%)
LIBS = -lws2_32

all: $(EXECS)

clean: rm $(EXECS)