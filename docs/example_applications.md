# Example Applications

This document details the example applications provided with the Classical RNG library, demonstrating practical use cases for both the Game RNG and Cryptographic RNG implementations.

## Game RNG Examples

### 1. Terrain Generation
Location: `examples/game/terrain_generation.c`

A heightmap-based terrain generator that creates realistic-looking landscapes using the Game RNG.

#### Features
- 256x256 resolution heightmap generation
- Island-like formation with center-based attenuation
- Outputs as PGM (Portable Gray Map) format
- Smooth noise function for natural-looking results

#### Usage
```bash
./build/terrain_gen > terrain.pgm
```

#### Implementation Details
- Uses dual random values for height generation
- Implements distance-based height attenuation
- Normalizes heights to 0-255 range for PGM output
- Memory-efficient single-pass generation

### 2. Particle System
Location: `examples/game/particle_system.c`

A 2D particle system simulation demonstrating the Game RNG's use in real-time applications.

#### Features
- Supports up to 1000 simultaneous particles
- Physics-based movement with gravity
- Variable particle lifetime
- Frame-by-frame state output

#### Usage
```bash
./build/particle_sys
```

#### Implementation Details
- Random velocity vector generation
- Particle lifetime randomization
- Efficient particle pool management
- Physics-based updates at 60 FPS

## Crypto RNG Examples

### 1. Token Generation
Location: `examples/crypto/token_generation.c`

A secure token generator for creating cryptographically strong random tokens with timestamps.

#### Features
- 256-bit token generation
- Timestamp integration
- Hexadecimal output format
- Configurable batch generation

#### Usage
```bash
# Generate a single token
./build/token_gen

# Generate multiple tokens in hex format
./build/token_gen --count 5 --hex
```

#### Security Properties
- Cryptographically secure random generation
- Timestamp-based uniqueness
- Prime number-based mixing
- Configurable mixing rounds

### 2. Key Derivation
Location: `examples/crypto/key_derivation.c`

A secure key derivation function (KDF) implementation for password-based key generation.

#### Features
- Variable key length (128-512 bits)
- Configurable iteration count
- 128-bit random salt generation
- Password-based derivation

#### Usage
```bash
# Generate a 256-bit key
./build/key_derive --password "your_password" --length 32

# Generate with custom iterations and show salt
./build/key_derive --password "your_password" --length 32 --iterations 20000 --show-salt
```

#### Security Properties
- Random salt generation
- Iterative mixing for computational hardness
- State-based mixing with password and salt
- Configurable security parameters

## Building the Examples

All examples can be built using the provided Makefile:

```bash
# Build all examples
make all

# Build specific examples
gcc -o build/terrain_gen examples/game/terrain_generation.c src/game_rng/game_rng.c -I. -lm
gcc -o build/particle_sys examples/game/particle_system.c src/game_rng/game_rng.c -I. -lm
gcc -o build/token_gen examples/crypto/token_generation.c src/crypto_rng/crypto_rng.c -I. -lm
gcc -o build/key_derive examples/crypto/key_derivation.c src/crypto_rng/crypto_rng.c -I. -lm
```

## Performance Considerations

### Game RNG Applications
- Terrain generation: ~0.1s for 256x256 heightmap
- Particle system: Supports 1000+ particles at 60 FPS
- Memory usage: < 1MB for both applications

### Crypto RNG Applications
- Token generation: ~10,000 tokens/second
- Key derivation: ~0.1s for 10,000 iterations
- Memory usage: < 1MB for both applications

## Best Practices

1. **Game Applications**
   - Use batch generation when possible
   - Reuse RNG instance for multiple generations
   - Consider thread-local RNG instances for parallel generation

2. **Cryptographic Applications**
   - Always use fresh salt for key derivation
   - Store salt alongside derived keys
   - Use appropriate iteration counts for target platform
   - Validate input parameters

## Example Output Samples

### Terrain Generation
```
P2
256 256
255
... [heightmap data in PGM format]
```

### Particle System
```
Frame 0
Active particles: 10
Particle 0: pos=(0.00, 0.00) vel=(1.23, 0.45) life=2.50
...
```

### Token Generation
```
65a1fc8e00000000-f7c91b3e4a2d8f5e9b0c7d2a1f6e4b8a...
```

### Key Derivation
```
Derived Key (32 bytes):
a1b2c3d4e5f6...
Salt:
f7e6d5c4b3a2...
