CC ?= clang
CFLAGS += -Wall -Wextra -std=gnu11 -g

SRC_DIR = src
BIN_DIR = bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
APPS := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(SRCS))

.PHONY: all clean

all: $(APPS)

$(BIN_DIR)/%: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

