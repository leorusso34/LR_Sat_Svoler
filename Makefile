CXX      := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++20
LDFLAGS  := 
SRC_DIR  := src
BUILD_DIR := build
EXEC     := sat_solver

SRCS := $(wildcard $(SRC_DIR)/*.cc)
OBJS := $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SRCS))

all: $(EXEC)

$(EXEC): $(OBJS)
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up"
	@rm -rf $(BUILD_DIR)/*.o $(EXEC)

.PHONY: all clean

run:
	./$(EXEC) prova.cnf
