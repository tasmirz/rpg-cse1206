# RPG world generator — Perlin noise + cellular automata, chunked open world.

COMPILER      = g++
STD           = -std=gnu++2b
WARNINGS      = -Wall -Wextra -Wno-long-long
DEBUG_FLAGS   = -g3
SRC_DIR       = src
BUILD_DIR     = build
PROJECT       = rpg

SRCS = $(wildcard $(SRC_DIR)/*.cxx)
OBJS = $(patsubst $(SRC_DIR)/%.cxx,$(BUILD_DIR)/%.o,$(SRCS))
BIN  = $(BUILD_DIR)/$(PROJECT)

all: $(BIN)

$(BIN): $(OBJS)
	$(COMPILER) $(STD) $(WARNINGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx | $(BUILD_DIR)
	$(COMPILER) $(STD) $(WARNINGS) $(DEBUG_FLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(BIN)
	$(BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
