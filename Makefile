CC = gcc
CFLAGS = -Wall -Werror -std=c2x -pedantic
TARGET = poker
LIBS = -lm

.PHONY: release debug clean default

release: $(TARGET)
release: CFLAGS += -g0 -O3 -flto
debug: $(TARGET)
debug: CFLAGS += -g3
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

