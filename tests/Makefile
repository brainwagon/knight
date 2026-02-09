CC = gcc
CFLAGS = -Wall -Wextra -g -I../src
LDFLAGS = -lm

SRC = test_constellation.c ../src/core.c ../src/constellation.c
OBJ = $(SRC:.c=.o)
TARGET = test_constellation

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
