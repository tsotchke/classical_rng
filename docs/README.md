# Documentation

Start with the question you are trying to answer:

| Question | Read |
|---|---|
| How do high-precision pi/e residues enter each draw? | [Mathematical principles](mathematical_principles.md) and [implementation details](implementation_details.md) |
| How do I build and call the library? | [Getting started](getting-started.md) |
| What does each function return or consume? | [API reference](api-reference.md) |
| Which properties are guarantees, conditional proofs, or non-claims? | [Guarantees](guarantees.md) |
| How is the deterministic stream specified? | [Deterministic PRNG](deterministic-prng.md) |
| What is safe for tokens and keys? | [Secure randomness](secure-randomness.md) |
| What experiments can I run to learn this? | [Learning guide](learning-guide.md) |
| Which compilers and operating systems are supported? | [Portability](portability.md) |
| How should I interpret bit-frequency tests? | [Bit distribution](bit_distribution.md) |
| How should the numerical cost be benchmarked? | [Performance analysis](performance_analysis.md) |
| Which sources support the algorithms and platform APIs? | [References](references.md) |

## Reading paths

Library integrators should read [Getting started](getting-started.md), the
[API reference](api-reference.md), [Guarantees](guarantees.md), and
[Portability](portability.md).

Readers auditing the deterministic construction should proceed through
[Mathematical principles](mathematical_principles.md), the
[deterministic specification](deterministic-prng.md), and
[implementation details](implementation_details.md), then compare the equations
with known-answer tests.

Security reviewers should read [Secure randomness](secure-randomness.md),
[Guarantees](guarantees.md), and the native-facility entries in
[References](references.md). The high-precision generator is intentionally
outside that security boundary.

## Executable examples

- [`examples/dice.c`](../examples/dice.c) shows reproducibility and bounded sampling.
- [`examples/constant_roundoff.c`](../examples/constant_roundoff.c) exposes exact pi/e remainders.
- [`examples/shuffle.c`](../examples/shuffle.c) implements an unbiased Fisher-Yates shuffle.
- [`examples/secure_token.c`](../examples/secure_token.c) obtains a 256-bit token from the OS.

Documentation distinguishes mathematical identities, implementation
contracts, empirical evidence, and open claims. A frequency plot can reveal a
broken implementation, but it cannot prove that a generator is unpredictable
or cryptographically secure.
