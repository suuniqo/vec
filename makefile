
### variables ###

debug ?= 0
NAME := vec
SRC_DIR := src
LIB_DIR := lib
BUILD_DIR := build
TESTS_DIR := tests

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o, $(SRCS))


### compiler ###

CC := gcc
CFLAGS := -Wall -Wextra -pedantic -Werror

ifeq ($(debug), 1)
	CFLAGS += -g -O0
else
	CFLAGS += $(CFLAGS) -O3
endif


### targets ###

all: dir $(NAME)

$(NAME): $(OBJS) dir
	@$(CC) $(CFLAGS) -shared $(OBJS) -o $(LIB_DIR)/lib$@.so


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c dir
	@$(CC) $(CFLAGS) -fPIC -c $< -o $@


### directory opts ###

dir:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)
	@rsync -a --include '*/' --exclude '*' $(SRC_DIR)/ $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR)
