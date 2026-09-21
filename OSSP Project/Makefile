# Compiler and compilation flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude -Isrc -O2
ASAN_FLAGS = -fsanitize=address,undefined -g

TARGET = scheduler
TEST_TARGET = test_runner

# Directories
SRC_DIR = src
INCLUDE_DIR = include
LOGS_DIR = logs
TESTS_DIR = tests

# Source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/worker.c $(SRC_DIR)/task_queue.c $(SRC_DIR)/logger.c $(SRC_DIR)/race_demo.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = $(TESTS_DIR)/test_suite.c $(SRC_DIR)/task_queue.c

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(LOGS_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build successful: ./$(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

demo: $(TARGET)
	./$(TARGET) --race-demo

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRCS)
	@echo "Test suite built: ./$(TEST_TARGET)"

asan:
	@mkdir -p $(LOGS_DIR)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $(TARGET)_asan $(SRCS)
	@echo "Running with AddressSanitizer and UndefinedBehaviorSanitizer..."
	./$(TARGET)_asan
	./$(TARGET)_asan --race-demo
	@rm -f $(TARGET)_asan

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) $(TEST_TARGET) $(TARGET)_asan scheduler_asan
	rm -rf $(LOGS_DIR)
	@echo "Cleaned all build artifacts and logs."

.PHONY: all run demo test asan clean
