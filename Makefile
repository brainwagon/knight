CC = gcc
NVCC = nvcc
CFLAGS = -Wall -Wextra -O3 -g -Isrc
LDFLAGS = -lm -ljpeg

# Check for nvcc
HAS_NVCC := $(shell command -v nvcc 2> /dev/null)

# Fallback to standard location
ifeq ($(HAS_NVCC),)
  ifneq ($(wildcard /usr/local/cuda/bin/nvcc),)
    HAS_NVCC = /usr/local/cuda/bin/nvcc
    NVCC = /usr/local/cuda/bin/nvcc
  endif
endif

# User can force enable/disable with CUDA=1 or CUDA=0
ifdef CUDA
  ifeq ($(CUDA), 1)
    ENABLE_CUDA = 1
  endif
else
  ifneq ($(HAS_NVCC),)
    ENABLE_CUDA = 1
  endif
endif

ifdef ENABLE_CUDA
    CFLAGS += -DCUDA_ENABLED
    # LDFLAGS += -lcudart # nvcc adds this automatically usually
    CU_SRC = src/render_cuda.cu
    CU_OBJ = $(CU_SRC:.cu=.o)
    LINK = $(NVCC)
else
    CU_OBJ =
    LINK = $(CC)
endif

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o) $(CU_OBJ)
TARGET = knight

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LINK) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cu
	$(NVCC) -O3 -Isrc -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
