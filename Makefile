CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
INCLUDES = -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
LEXER_SRC = $(SRC_DIR)/lexer/lexer.c
PARSER_SRC = $(SRC_DIR)/parser/parser.c
AST_SRC = $(SRC_DIR)/ast/ast.c
SEMANTIC_SRC = $(SRC_DIR)/semantic/semantic.c $(SRC_DIR)/semantic/symbol_table.c
CODEGEN_SRC = $(SRC_DIR)/codegen/codegen.c
MAIN_SRC = $(SRC_DIR)/main.c

# Object files
LEXER_OBJ = $(OBJ_DIR)/lexer.o
PARSER_OBJ = $(OBJ_DIR)/parser.o
AST_OBJ = $(OBJ_DIR)/ast.o
SEMANTIC_OBJ = $(OBJ_DIR)/semantic.o $(OBJ_DIR)/symbol_table.o
CODEGEN_OBJ = $(OBJ_DIR)/codegen.o
MAIN_OBJ = $(OBJ_DIR)/main.o

# All object files
OBJS = $(LEXER_OBJ) $(PARSER_OBJ) $(AST_OBJ) $(SEMANTIC_OBJ) $(CODEGEN_OBJ) $(MAIN_OBJ)

# Binary name
BIN = $(BIN_DIR)/hindic

# Default target
all: directories $(BIN)

# Create directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Link the binary
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile lexer
$(LEXER_OBJ): $(LEXER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile parser
$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile AST
$(AST_OBJ): $(AST_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile semantic analyzer
$(OBJ_DIR)/semantic.o: $(SRC_DIR)/semantic/semantic.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/symbol_table.o: $(SRC_DIR)/semantic/symbol_table.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile code generator
$(CODEGEN_OBJ): $(CODEGEN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile main
$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Test with an example
test: all
	@echo "Testing with example program..."
	$(BIN) examples/hello.hc -o examples/hello.c
	gcc -o examples/hello examples/hello.c
	@echo "Running the example program:"
	examples/hello

.PHONY: all clean test directories