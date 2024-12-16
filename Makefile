CC = gcc
CFLAGS = -O3 -Wall -Wextra -I./src -I./tests -I./tests/test_utils -I./
LDFLAGS = -lm

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests

GAME_RNG_SRCS = $(SRC_DIR)/game_rng/game_rng.c
CRYPTO_RNG_SRCS = $(SRC_DIR)/crypto_rng/crypto_rng.c
TEST_SRCS = $(TEST_DIR)/test_utils/statistical_tests.c

GAME_RNG_OBJS = $(BUILD_DIR)/game_rng.o
CRYPTO_RNG_OBJS = $(BUILD_DIR)/crypto_rng.o
TEST_UTILS_OBJS = $(BUILD_DIR)/statistical_tests.o

BINS = $(BUILD_DIR)/game_rng_cli \
       $(BUILD_DIR)/crypto_rng_cli \
       $(BUILD_DIR)/test_game_rng \
       $(BUILD_DIR)/test_crypto_rng

.PHONY: all clean test

all: $(BUILD_DIR) $(BINS)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/*/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@

$(BUILD_DIR)/statistical_tests.o: $(TEST_DIR)/test_utils/statistical_tests.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/game_rng_cli: $(SRC_DIR)/game_rng/game_rng_cli.c $(GAME_RNG_OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/crypto_rng_cli: $(SRC_DIR)/crypto_rng/crypto_rng_cli.c $(CRYPTO_RNG_OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/test_game_rng: $(TEST_DIR)/test_game_rng.c $(GAME_RNG_OBJS) $(TEST_UTILS_OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/test_crypto_rng: $(TEST_DIR)/test_crypto_rng.c $(CRYPTO_RNG_OBJS) $(TEST_UTILS_OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

test: $(BUILD_DIR)/test_game_rng $(BUILD_DIR)/test_crypto_rng
	./$(BUILD_DIR)/test_game_rng
	./$(BUILD_DIR)/test_crypto_rng

viz: test
	@mkdir -p build
	@echo "Running tests and generating visualization data..."
	./$(BUILD_DIR)/test_game_rng > build/game_rng_results.json
	./$(BUILD_DIR)/test_crypto_rng > build/crypto_rng_results.json
	cd visualization && npm start