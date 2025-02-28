CC = gcc
CFLAGS = -Wall -Werror -std=c2x -pedantic 
TARGET = gen_tables
LIBS = -lm -lcmph 

.PHONY: release clean default debug

release: $(TARGET)
release: CFLAGS += -O3
debug: $(TARGET)
debug: CFLAGS += -O0 -g3
default: release

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	- rm -f *.o
	- rm -f $(TARGET)

