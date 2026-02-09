CC = gcc
CFLAGS = -Wall -Wextra -g -I../src
LDFLAGS = -lm

SRC = test_constellation.c ../src/core.c ../src/constellation.c ../src/ephemerides.c ../src/stars.c
OBJ = $(SRC:.c=.o)
TARGET = test_constellation
CONFIG_TARGET = test_config
LABEL_CONFIG_TARGET = test_label_config
LABELS_TARGET = test_labels
ENV_PROJ_TARGET = test_env_proj
MATH_TARGET = test_math
PSF_TARGET = test_psf

DIAG_SRC = diagnostic_projection.c ../src/core.c ../src/constellation.c ../src/ephemerides.c ../src/stars.c
DIAG_OBJ = $(DIAG_SRC:.c=.o)
DIAG_TARGET = diagnostic_projection

all: $(TARGET) $(CONFIG_TARGET) $(LABEL_CONFIG_TARGET) $(LABELS_TARGET) $(ENV_PROJ_TARGET) $(MATH_TARGET) $(PSF_TARGET)
	./$(TARGET)
	./$(CONFIG_TARGET)
	./$(LABEL_CONFIG_TARGET)
	./$(LABELS_TARGET)
	./$(ENV_PROJ_TARGET)
	./$(MATH_TARGET)
	./$(PSF_TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(CONFIG_TARGET): test_config.o ../src/config.o
	$(CC) test_config.o ../src/config.o -o $(CONFIG_TARGET) $(LDFLAGS)

$(LABEL_CONFIG_TARGET): test_label_config.o ../src/config.o ../src/core.o
	$(CC) test_label_config.o ../src/config.o ../src/core.o -o $(LABEL_CONFIG_TARGET) $(LDFLAGS)

$(LABELS_TARGET): test_labels.o ../src/constellation.o ../src/core.o ../src/ephemerides.o ../src/stars.o ../src/tonemap.o ../src/image.o
	$(CC) test_labels.o ../src/constellation.o ../src/core.o ../src/ephemerides.o ../src/stars.o ../src/tonemap.o ../src/image.o -o $(LABELS_TARGET) $(LDFLAGS) -ljpeg

$(ENV_PROJ_TARGET): test_env_proj.o ../src/constellation.o ../src/core.o ../src/ephemerides.o ../src/stars.o ../src/tonemap.o ../src/image.o
	$(CC) test_env_proj.o ../src/constellation.o ../src/core.o ../src/ephemerides.o ../src/stars.o ../src/tonemap.o ../src/image.o -o $(ENV_PROJ_TARGET) $(LDFLAGS) -ljpeg

$(MATH_TARGET): test_math.o ../src/core.o
	$(CC) test_math.o ../src/core.o -o $(MATH_TARGET) $(LDFLAGS)

$(PSF_TARGET): test_psf.o ../src/stars.o ../src/core.o ../src/image.o ../src/tonemap.o
	$(CC) test_psf.o ../src/stars.o ../src/core.o ../src/image.o ../src/tonemap.o -o $(PSF_TARGET) $(LDFLAGS) -ljpeg

$(DIAG_TARGET): $(DIAG_OBJ)
	$(CC) $(DIAG_OBJ) -o $(DIAG_TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(CONFIG_TARGET) $(TARGET) $(DIAG_TARGET) test_config.o ../src/config.o

.PHONY: all clean