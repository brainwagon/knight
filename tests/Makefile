CC = gcc
CFLAGS = -Wall -Wextra -g -I../src
LDFLAGS = -lm

SRC = test_constellation.c ../src/core.c ../src/constellation.c ../src/ephemerides.c ../src/stars.c
OBJ = $(SRC:.c=.o)
TARGET = test_constellation
CONFIG_TARGET = test_config
MATH_TARGET = test_math

DIAG_SRC = diagnostic_projection.c ../src/core.c ../src/constellation.c ../src/ephemerides.c ../src/stars.c
DIAG_OBJ = $(DIAG_SRC:.c=.o)
DIAG_TARGET = diagnostic_projection

all: $(TARGET) $(CONFIG_TARGET) $(MATH_TARGET)
	./$(TARGET)
	./$(CONFIG_TARGET)
	./$(MATH_TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(CONFIG_TARGET): test_config.o ../src/config.o
	$(CC) test_config.o ../src/config.o -o $(CONFIG_TARGET) $(LDFLAGS)

$(MATH_TARGET): test_math.o ../src/core.o
	$(CC) test_math.o ../src/core.o -o $(MATH_TARGET) $(LDFLAGS)

$(DIAG_TARGET): $(DIAG_OBJ)
	$(CC) $(DIAG_OBJ) -o $(DIAG_TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(CONFIG_TARGET) $(TARGET) $(DIAG_TARGET) test_config.o ../src/config.o

.PHONY: all clean