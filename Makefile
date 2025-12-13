CC = gcc
CFLAGS = -O3 -lm -Wall -Wextra
INCLUDE = -Iinclude

SOURCES = src/main.c src/linalg.c src/nn.c src/gan.c
TARGET = gan_train

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

. PHONY: all clean run