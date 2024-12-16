# Mathematical Principles of Classical RNG

This document details the mathematical foundations underlying both the Game RNG and Cryptographic RNG implementations.

## Game RNG Mathematical Foundation

### Mathematical Constants Integration

The Game RNG utilizes two fundamental mathematical constants:

1. **π (Pi)**: Used for its infinite, non-repeating decimal sequence
   - Implemented as a FastBigInt for efficient computation
   - Provides natural entropy source for mixing functions
   - Decimal expansion used in state transitions

2. **e (Euler's number)**: Complements π in the mixing process
   - Provides additional irrational number sequences
   - Used in rotation calculations
   - Ensures non-linear state transitions

### Rotation-Based Mixing

The mixing function employs prime number-based rotations:

```c
state[0] = (state[0] << rotation_primes[0]) | (state[0] >> (64 - rotation_primes[0]));
state[1] = (state[1] << rotation_primes[1]) | (state[1] >> (64 - rotation_primes[1]));
```

Properties:
- Maintains uniform distribution
- Preserves bit entropy
- Fast execution on modern CPUs
- Reversible operation (important for testing)

### State Transition Function

The state transition combines:
1. Bitwise operations
2. Mathematical constant mixing
3. Prime number rotations

Expressed mathematically as:
```
S[t+1] = R(S[t] ⊕ π[i]) ⊗ R(S[t] ⊕ e[i])
```
where:
- S[t] is the state at time t
- R() is the rotation function
- ⊕ is bitwise XOR
- ⊗ is the mixing operation
- π[i], e[i] are digit sequences

## Cryptographic RNG Mathematical Foundation

### Entropy Accumulation

The entropy pool uses:
1. Hardware-based entropy sources
2. Environmental entropy (timestamps, process IDs)
3. Mathematical mixing functions

Entropy is accumulated using:
```
E[t+1] = H(E[t] || S || T)
```
where:
- E[t] is entropy state at time t
- H() is a mixing function
- S is system entropy
- T is timestamp
- || denotes concatenation

### Prime Number Theory Application

The cryptographic RNG heavily utilizes prime number theory:

1. **Prime Generation**
   - Uses Miller-Rabin primality testing
   - Implements optimized sieving
   - Ensures cryptographic strength

2. **Prime Verification**
   ```
   is_prime(n, rounds) = ∏(witness_test(n, a[i]))
   ```
   where a[i] are carefully chosen witness values

### Security Properties

Mathematical foundations ensuring security:

1. **Forward Secrecy**
   - State transitions are non-reversible
   - Previous states cannot be derived from current state

2. **Prediction Resistance**
   - Uses entropy pooling
   - Implements mixing rounds
   - Ensures output unpredictability

3. **Distribution Properties**
   - Uniform distribution across output range
   - High entropy per bit
   - No statistical biases

## Statistical Foundations

### Chi-Square Test Implementation

The chi-square test is implemented as:
```
χ² = ∑((O[i] - E[i])² / E[i])
```
where:
- O[i] are observed frequencies
- E[i] are expected frequencies

### Bit Entropy Calculation

Bit entropy is calculated using:
```
H = -∑(p[i] * log₂(p[i]))
```
where p[i] are bit probabilities

### Sequence Correlation Analysis

Correlation coefficient calculation:
```
r = cov(X,Y) / (σ[X] * σ[Y])
```
where:
- cov(X,Y) is sequence covariance
- σ[X], σ[Y] are standard deviations

## Performance Optimization Principles

### Fast BigInt Operations

Optimized big integer operations use:
1. Word-aligned storage
2. Cache-friendly algorithms
3. SIMD-friendly data structures

### Mixing Function Efficiency

The mixing functions are designed for:
1. CPU pipeline optimization
2. Minimal branch predictions
3. Efficient register usage

## References

1. Knuth, Donald E. "The Art of Computer Programming, Volume 2: Seminumerical Algorithms"
2. Marsaglia, George. "Random number generation"
3. L'Ecuyer, Pierre. "Testing Random Number Generators"
4. NIST Special Publication 800-90A: "Recommendation for Random Number Generation Using Deterministic Random Bit Generators"
