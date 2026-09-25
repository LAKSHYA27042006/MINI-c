CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -Isrc

SRCS = src/error.c \
       src/token.c \
       src/lexer.c \
       src/ast.c \
       src/parser.c \
       src/symbol_table.c \
       src/semantic.c \
       src/tac.c \
       src/optimizer.c

MAIN_SRC = src/main.c
TEST_SRC = tests/test_runner.c

ifeq ($(OS),Windows_NT)
    TARGET_BIN = minic.exe
    TEST_BIN = test_runner.exe
    RM = -cmd /C del /Q /F
else
    TARGET_BIN = minic
    TEST_BIN = test_runner
    RM = rm -f
endif

.PHONY: all clean run test

all: $(TARGET_BIN)

$(TARGET_BIN): $(SRCS) $(MAIN_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_BIN): $(SRCS) $(TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET_BIN)
	./$(TARGET_BIN) examples/basic.mc --all

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	$(RM) $(TARGET_BIN) $(TEST_BIN) 2>nul || exit 0
