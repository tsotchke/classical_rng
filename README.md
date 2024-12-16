# Classical RNG Library

A high-performance C library providing two specialized random number generators: a fast game-oriented RNG and a cryptographically secure RNG. Each implementation is optimized for its specific use case while maintaining high statistical quality.

## 🎮 Game RNG

The Game RNG implementation uses a hybrid approach combining mathematical constants (π and e) with rotation-based mixing functions for fast, high-quality random number generation.

### Features
- Ultra-fast generation (~36M numbers/second)
- Excellent statistical distribution (χ² ≈ 1000)
- Perfect bit entropy (2.0)
- Optimized for real-time applications

### CLI Usage
```bash
# Basic usage (generates 10 random numbers)
./build/game_rng_cli

# Generate 100 random floating-point numbers between 0 and 1
./build/game_rng_cli --num_values 100 --float

# Generate numbers within a specific range (e.g., for dice rolls)
./build/game_rng_cli --num_values 50 --range_min 1 --range_max 6

# Generate hexadecimal output
./build/game_rng_cli --format hex --num_values 20

# Run performance benchmark
./build/game_rng_cli --benchmark

# Custom benchmark size
./build/game_rng_cli --benchmark --benchmark_size 50000000

# Quiet mode (suppress additional output)
./build/game_rng_cli --quiet --num_values 10
```

### CLI Arguments (Game RNG)
```
--num_values N     Number of random values to generate (default: 10)
                  Range: 1 to MAX_INT
                  Example: --num_values 100

--format FORMAT    Output format for generated numbers
                  Values: decimal (default), hex
                  Example: --format hex

--range_min N     Minimum value for range output (default: 0)
                  Must be less than range_max
                  Example: --range_min 1

--range_max N     Maximum value for range output
                  Must be greater than range_min
                  Example: --range_max 6

--float           Output as floating point numbers [0,1]
                  No additional parameters needed
                  Example: --float

--benchmark       Run performance benchmark
                  Cannot be combined with other generation options
                  Example: --benchmark

--benchmark_size N Number of iterations for benchmark
                  Default: 10000000
                  Example: --benchmark_size 50000000

--quiet           Suppress additional output
                  Only show generated numbers
                  Example: --quiet

--help            Display help message with all options
```

## 🔒 Crypto RNG

The Cryptographic RNG implementation focuses on security-critical applications requiring unpredictable random numbers.

### Features
- Cryptographically secure output
- Hardware entropy source integration
- Prime number generation
- Configurable mixing rounds

### CLI Usage
```bash
# Basic usage (generates 10 random numbers)
./build/crypto_rng_cli

# Generate with custom prime bounds and mixing rounds
./build/crypto_rng_cli --prime_lower 1000 --prime_upper 10000 --mixing_rounds 30

# Generate 50 numbers in hexadecimal format
./build/crypto_rng_cli --num_values 50 --format hex

# Run performance benchmark
./build/crypto_rng_cli --benchmark

# Custom benchmark with specific parameters
./build/crypto_rng_cli --benchmark --benchmark_size 100000 --mixing_rounds 40

# Quiet mode with hex output
./build/crypto_rng_cli --quiet --format hex --num_values 5
```

### CLI Arguments (Crypto RNG)
```
--num_values N     Number of random values to generate (default: 10)
                  Range: 1 to MAX_INT
                  Example: --num_values 50

--prime_lower N    Lower bound for prime numbers
                  Must be >= 2
                  Default: 2^(MIN_PRIME_BITS-1)
                  Example: --prime_lower 1000

--prime_upper N    Upper bound for prime numbers
                  Must be > prime_lower
                  Default: 2^MIN_PRIME_BITS
                  Example: --prime_upper 10000

--mixing_rounds N  Number of mixing rounds for extra security
                  Range: 1 to MAX_INT
                  Default: DEFAULT_MIXING_ROUNDS (20)
                  Example: --mixing_rounds 30

--format FORMAT    Output format for generated numbers
                  Values: decimal (default), hex
                  Example: --format hex

--benchmark       Run performance benchmark
                  Cannot be combined with other generation options
                  Example: --benchmark

--benchmark_size N Number of iterations for benchmark
                  Default: 10000000
                  Example: --benchmark_size 100000

--quiet           Suppress additional output
                  Only show generated numbers
                  Example: --quiet

--help            Display help message with all options
```

## 📊 Example Applications

### Game Development
```c
// Terrain Generation
GameRNG rng;
init_game_rng(&rng);
float height = random_float(&rng) * MAX_HEIGHT;

// Particle System
float angle = random_float(&rng) * 2 * M_PI;
float speed = random_float(&rng) * 2.0f + 1.0f;
```

### Security Applications
```c
// Token Generation
uint64_t token = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS);

// Key Derivation
uint8_t key[32];
derive_key(password, sizeof(key), iterations, &key);
```

See [Example Applications](docs/example_applications.md) for more detailed examples.

## 🛠 Building

### Prerequisites
- C compiler (gcc/clang)
- Make
- Node.js (for visualization, optional)

### Building
```bash
# Build everything (libraries, CLIs, and tests)
make all

# Run tests
make test

# Run tests with visualization
make viz
```

## 📈 Testing

### Running Tests
```bash
# Basic test suite
./build/test_game_rng
./build/test_crypto_rng

# With visualization
cd tests/visualization
npm install
npm start
```

### Test Output
```json
{
  "distribution": [...],
  "bit_counts": [...],
  "transition_matrix": [[...], [...]],
  "metrics": {
    "chi_square": 1004.852,
    "bit_entropy": 2.000000,
    "generation_time": 0.027543,
    "numbers_per_second": 36306865
  }
}
```

## 📚 Documentation

- [Mathematical Principles](docs/mathematical_principles.md)
- [Performance Analysis](docs/performance_analysis.md)
- [Bit Distribution](docs/bit_distribution.md)
- [Implementation Details](docs/implementation_details.md)
- [Example Applications](docs/example_applications.md)

## 📁 Project Structure

```
classical_rng/
├── docs/                           # Documentation
│   ├── mathematical_principles.md  # Mathematical foundations
│   ├── performance_analysis.md     # Performance benchmarks and analysis
│   ├── bit_distribution.md        # Statistical properties analysis
│   ├── implementation_details.md   # Implementation specifics
│   └── example_applications.md     # Detailed example usage
│
├── src/                           # Source code
│   ├── common/                    # Shared utilities
│   │   └── constants.h           # Common constants and configurations
│   │
│   ├── game_rng/                 # Game RNG implementation
│   │   ├── game_rng.h           # Public API
│   │   ├── game_rng.c           # Implementation
│   │   └── game_rng_cli.c       # Command-line interface
│   │
│   └── crypto_rng/               # Cryptographic RNG implementation
│       ├── crypto_rng.h         # Public API
│       ├── crypto_rng.c         # Implementation
│       └── crypto_rng_cli.c     # Command-line interface
│
├── tests/                         # Test suite
│   ├── test_game_rng.c          # Game RNG tests
│   ├── test_game_rng.h          # Game RNG test headers
│   ├── test_crypto_rng.c        # Crypto RNG tests
│   ├── test_crypto_rng.h        # Crypto RNG test headers
│   │
│   ├── test_utils/              # Test utilities
│   │   ├── statistical_tests.c  # Statistical test implementations
│   │   └── statistical_tests.h  # Statistical test headers
│   │
│   └── visualization/           # Test result visualization
│       ├── src/                # React application source
│       ├── package.json        # Node.js dependencies
│       └── vite.config.js      # Vite configuration
│
├── examples/                      # Example applications
│   ├── game/                    # Game RNG examples
│   │   ├── terrain_generation.c # Terrain generation example
│   │   └── particle_system.c    # Particle system example
│   │
│   └── crypto/                  # Crypto RNG examples
│       ├── token_generation.c   # Token generation example
│       └── key_derivation.c     # Key derivation example
│
├── Makefile                      # Build configuration
├── .gitignore                    # Git ignore rules
├── LICENSE                       # MIT License
├── CONTRIBUTING.md               # Contribution guidelines
└── README.md                     # This file
```

## 🔍 Implementation Details

### Game RNG
```c
typedef struct {
    uint64_t state[4];
    FastBigInt pi;
    FastBigInt e;
    uint64_t rotation_primes[8];
} GameRNG;
```

### Crypto RNG
```c
uint64_t secure_random(uint64_t prime_lower, 
                      uint64_t prime_upper, 
                      int mixing_rounds);
```

## 🧪 Statistical Properties

### Game RNG
- Distribution: Uniform
- Period: 2^256
- Chi-square: ~1004.85
- Bit entropy: 2.0

### Crypto RNG
- NIST SP 800-22 compliant
- Cryptographically secure
- Non-deterministic
- Hardware entropy integration

## 🚀 Performance

### Game RNG
- ~36M numbers/second
- ~27.5ns per number
- Cache-friendly design
- SIMD optimizations

### Crypto RNG
- ~50K secure numbers/second
- Configurable security levels
- Memory-hardened
- Side-channel resistant

## Citation
If you use this project in your research, please cite as follows:

```bibtex
@software{ClassicalRNG,
  author = {tsotchke},
  title = {Classical Random Number Generators},
  year = {2024},
  url = {https://github.com/tsotchke/classical_rng}
}
```

## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and submission process.
