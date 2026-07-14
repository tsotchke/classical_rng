CC ?= cc
AR ?= ar
RANLIB ?= ranlib
PREFIX ?= /usr/local
BUILD_DIR ?= build

CPPFLAGS ?=
CFLAGS ?= -O2
LDFLAGS ?=
LDLIBS ?=

CPPFLAGS += -Iinclude
CRNG_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
LDLIBS += -lbcrypt
endif

LIB_SOURCES := src/game_rng/game_rng.c src/crypto_rng/crypto_rng.c src/common/status.c
LIB_OBJECTS := $(BUILD_DIR)/game_rng.o $(BUILD_DIR)/crypto_rng.o $(BUILD_DIR)/status.o
LIBRARY := $(BUILD_DIR)/libclassical_rng.a
EXAMPLES := $(BUILD_DIR)/crng_constant_roundoff$(EXEEXT) \
            $(BUILD_DIR)/crng_dice$(EXEEXT) \
            $(BUILD_DIR)/crng_shuffle$(EXEEXT) \
            $(BUILD_DIR)/crng_secure_token$(EXEEXT)
TOOLS := $(BUILD_DIR)/game_rng_cli$(EXEEXT) $(BUILD_DIR)/crypto_rng_cli$(EXEEXT)
TEST_BINARY := $(BUILD_DIR)/crng_tests$(EXEEXT)
GAME_TEST_BINARY := $(BUILD_DIR)/test_game_rng$(EXEEXT)
CRYPTO_TEST_BINARY := $(BUILD_DIR)/test_crypto_rng$(EXEEXT)

.PHONY: all clean examples install sanitize test tools viz viz-data

all: $(LIBRARY) examples tools

examples: $(EXAMPLES)

tools: $(TOOLS)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/game_rng.o: src/game_rng/game_rng.c include/classical_rng/game_rng.h src/common/constants.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) -c $< -o $@

$(BUILD_DIR)/crypto_rng.o: src/crypto_rng/crypto_rng.c include/classical_rng/crypto_rng.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) -c $< -o $@

$(BUILD_DIR)/status.o: src/common/status.c include/classical_rng/common.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) -c $< -o $@

$(LIBRARY): $(LIB_OBJECTS)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(BUILD_DIR)/crng_%$(EXEEXT): examples/%.c $(LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) $< $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/game_rng_cli$(EXEEXT): src/game_rng/game_rng_cli.c $(LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) $< $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/crypto_rng_cli$(EXEEXT): src/crypto_rng/crypto_rng_cli.c $(LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) $< $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

$(TEST_BINARY): tests/test_classical_rng.c $(LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CRNG_CFLAGS) $< $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

$(GAME_TEST_BINARY): tests/test_game_rng.c tests/test_utils/statistical_tests.c $(LIBRARY)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) $(CRNG_CFLAGS) tests/test_game_rng.c tests/test_utils/statistical_tests.c $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

$(CRYPTO_TEST_BINARY): tests/test_crypto_rng.c tests/test_utils/statistical_tests.c $(LIBRARY)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) $(CRNG_CFLAGS) tests/test_crypto_rng.c tests/test_utils/statistical_tests.c $(LIBRARY) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TEST_BINARY) $(GAME_TEST_BINARY) $(CRYPTO_TEST_BINARY)
	$(TEST_BINARY)
	$(GAME_TEST_BINARY)
	$(CRYPTO_TEST_BINARY)

viz-data: $(GAME_TEST_BINARY) $(CRYPTO_TEST_BINARY)
	$(GAME_TEST_BINARY) > $(BUILD_DIR)/game_rng_results.json
	$(CRYPTO_TEST_BINARY) > $(BUILD_DIR)/crypto_rng_results.json

viz: viz-data
	npm --prefix tests/visualization run dev

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="-O1 -g -Werror -fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined" test

install: $(LIBRARY)
	mkdir -p $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
	cp include/classical_rng.h $(DESTDIR)$(PREFIX)/include/
	cp -R include/classical_rng $(DESTDIR)$(PREFIX)/include/
	cp $(LIBRARY) $(DESTDIR)$(PREFIX)/lib/

clean:
	rm -rf $(BUILD_DIR)
