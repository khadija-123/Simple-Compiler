CC = gcc
CFLAGS = -Wall -std=c99
TARGET = compiler
SOURCES = compiler.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET) test.src output.ir
	@echo "\n=== COMPILER OUTPUT ===\n"
	@cat output.ir

clean:
	rm -f $(OBJECTS) $(TARGET) output.ir

.PHONY: all test clean
