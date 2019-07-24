LIBS = -lm
CC = gcc
CFLAGS = -g -pthread 
SOURCES = dirlist.c threadpool.c fileparser.c maxheap.c

TARGET = ssfi

default: all

all: clean $(TARGET) 

$(TARGET):
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(TARGET)
	
