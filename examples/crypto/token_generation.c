#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "../../src/crypto_rng/crypto_rng.h"

#define TOKEN_LENGTH 32  // 256-bit tokens
#define MAX_TOKENS 1000

typedef struct {
    uint64_t timestamp;
    uint8_t token[TOKEN_LENGTH];
} SecureToken;

void generate_token(SecureToken* token) {
    // Use MIN_PRIME_BITS for both bounds to prevent overflow
    uint64_t prime_lower = 1ULL << (MIN_PRIME_BITS - 1);
    uint64_t prime_upper = (1ULL << MIN_PRIME_BITS) - 1;
    
    // Set timestamp
    token->timestamp = time(NULL);
    
    // Fill token with random bytes
    for (int i = 0; i < TOKEN_LENGTH; i += 8) {
        uint64_t random = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS);
        for (int j = 0; j < 8 && (i + j) < TOKEN_LENGTH; j++) {
            token->token[i + j] = (random >> (j * 8)) & 0xFF;
        }
    }
}

void token_to_hex(const SecureToken* token, char* hex_out) {
    char* ptr = hex_out;
    ptr += sprintf(ptr, "%016llx-", (unsigned long long)token->timestamp);
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        ptr += sprintf(ptr, "%02x", token->token[i]);
    }
}

int main(int argc, char* argv[]) {
    int num_tokens = 1;
    bool hex_format = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--count") == 0 && i + 1 < argc) {
            num_tokens = atoi(argv[i + 1]);
            if (num_tokens <= 0 || num_tokens > MAX_TOKENS) {
                fprintf(stderr, "Token count must be between 1 and %d\n", MAX_TOKENS);
                return 1;
            }
            i++;
        } else if (strcmp(argv[i], "--hex") == 0) {
            hex_format = true;
        } else {
            fprintf(stderr, "Usage: %s [--count N] [--hex]\n", argv[0]);
            return 1;
        }
    }
    
    // Generate and output tokens
    char hex_token[TOKEN_LENGTH * 2 + 19];  // Extra space for timestamp and hyphens
    
    for (int i = 0; i < num_tokens; i++) {
        SecureToken token;
        generate_token(&token);
        
        if (hex_format) {
            token_to_hex(&token, hex_token);
            printf("%s\n", hex_token);
        } else {
            printf("Token %d:\n", i + 1);
            printf("  Timestamp: %llu\n", (unsigned long long)token.timestamp);
            printf("  Data: ");
            for (int j = 0; j < TOKEN_LENGTH; j++) {
                printf("%02x", token.token[j]);
            }
            printf("\n\n");
        }
    }
    
    return 0;
}
