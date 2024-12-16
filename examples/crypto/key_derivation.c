#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../src/crypto_rng/crypto_rng.h"

#define MAX_KEY_LENGTH 64    // 512 bits maximum
#define MIN_KEY_LENGTH 16    // 128 bits minimum
#define SALT_LENGTH 16       // 128 bit salt
#define ITERATIONS 10000     // Default iteration count

typedef struct {
    uint8_t key[MAX_KEY_LENGTH];
    uint8_t salt[SALT_LENGTH];
    size_t key_length;
} DerivedKey;

void generate_salt(uint8_t* salt) {
    // Use MIN_PRIME_BITS for both bounds to prevent overflow
    uint64_t prime_lower = 1ULL << (MIN_PRIME_BITS - 1);
    uint64_t prime_upper = (1ULL << MIN_PRIME_BITS) - 1;
    
    for (int i = 0; i < SALT_LENGTH; i += 8) {
        uint64_t random = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS);
        for (int j = 0; j < 8 && (i + j) < SALT_LENGTH; j++) {
            salt[i + j] = (random >> (j * 8)) & 0xFF;
        }
    }
}

void derive_key(const char* password, size_t key_length, int iterations, DerivedKey* result) {
    if (key_length < MIN_KEY_LENGTH || key_length > MAX_KEY_LENGTH) {
        fprintf(stderr, "Key length must be between %d and %d bytes\n", 
                MIN_KEY_LENGTH, MAX_KEY_LENGTH);
        exit(1);
    }
    
    result->key_length = key_length;
    generate_salt(result->salt);
    
    // Use MIN_PRIME_BITS for both bounds to prevent overflow
    uint64_t prime_lower = 1ULL << (MIN_PRIME_BITS - 1);
    uint64_t prime_upper = (1ULL << MIN_PRIME_BITS) - 1;
    
    // Initial key material
    uint64_t state = 0;
    for (size_t i = 0; password[i]; i++) {
        state = (state << 8) | password[i];
        if ((i + 1) % 8 == 0) {
            state = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS) ^ state;
        }
    }
    
    // Mix with salt
    for (int i = 0; i < SALT_LENGTH; i++) {
        state ^= ((uint64_t)result->salt[i]) << ((i % 8) * 8);
        if ((i + 1) % 8 == 0) {
            state = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS) ^ state;
        }
    }
    
    // Iterative mixing
    for (int i = 0; i < iterations; i++) {
        state = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS) ^ state;
    }
    
    // Fill key buffer
    for (size_t i = 0; i < key_length; i += 8) {
        uint64_t random = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS) ^ state;
        for (size_t j = 0; j < 8 && (i + j) < key_length; j++) {
            result->key[i + j] = (random >> (j * 8)) & 0xFF;
        }
        state = random;
    }
}

void print_key(const DerivedKey* key, bool include_salt) {
    printf("Derived Key (%zu bytes):\n", key->key_length);
    for (size_t i = 0; i < key->key_length; i++) {
        printf("%02x", key->key[i]);
        if ((i + 1) % 32 == 0) printf("\n");
    }
    printf("\n");
    
    if (include_salt) {
        printf("Salt:\n");
        for (int i = 0; i < SALT_LENGTH; i++) {
            printf("%02x", key->salt[i]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    size_t key_length = 32;  // Default to 256 bits
    int iterations = ITERATIONS;
    bool show_salt = false;
    const char* password = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--length") == 0 && i + 1 < argc) {
            key_length = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--password") == 0 && i + 1 < argc) {
            password = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--show-salt") == 0) {
            show_salt = true;
        } else {
            fprintf(stderr, "Usage: %s --password PASS [--length N] [--iterations N] [--show-salt]\n", argv[0]);
            return 1;
        }
    }
    
    if (!password) {
        fprintf(stderr, "Password is required\n");
        return 1;
    }
    
    DerivedKey key;
    derive_key(password, key_length, iterations, &key);
    print_key(&key, show_salt);
    
    return 0;
}
