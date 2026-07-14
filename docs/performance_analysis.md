# Performance analysis

Version 2.0.0 ships no benchmark harness and makes no numeric throughput or
latency claim. This page identifies the cost model and a reproducible protocol
for future measurements.

## Cost model

One raw deterministic 64-bit draw performs:

- one xoshiro256** output and state transition;
- two exact divisions of 23 active base-`10^9` limbs by 32-bit divisors;
- fixed 64-bit divisor derivation and avalanche mixing.

The two long divisions are the deliberate numerical feature. Their work is
bounded and independent of the divisor magnitude. `crng_game_rng_fill`
amortizes one draw over at most eight output bytes but discards unused bytes in
a partial final word.

Bounded generation may reject source words. If the bound is `b` and
`t = 2^64 mod b`, expected draws are `1/(1 - t/2^64) < 2`. The exact rate
depends on `b` and must be reported with bounded benchmarks.

Secure APIs are dominated by native-system behavior: syscall or library-call
overhead, provider batching, blocking policy, and request size. Prime search
adds a data-dependent number of candidate draws and Miller-Rabin operations;
its latency distribution is more informative than one mean.

## Reproducible benchmark protocol

A defensible benchmark should:

1. identify CPU model, architecture, OS/kernel, power policy, compiler/version,
   optimization flags, link mode, and classical_rng revision;
2. benchmark Release artifacts while preserving the same semantics and
   known-answer vectors as tested builds;
3. warm instruction/data caches separately from recorded trials;
4. run enough independent trials to report median, dispersion, and preferably
   a confidence interval rather than one best observation;
5. use a monotonic high-resolution timer and measure an empty-loop or harness
   baseline;
6. consume results through a compiler-visible checksum or equivalent barrier
   so generation cannot be optimized away;
7. pin execution or record scheduling policy where reproducibility matters;
8. exclude allocation, terminal I/O, text formatting, and setup unless those
   are explicitly the subject of the measurement;
9. publish source, raw observations, and the aggregation procedure.

Clock-frequency scaling, thermal throttling, virtual machines, and shared
continuous-integration hosts can dominate small differences. Report them
rather than presenting cross-machine ratios as algorithm-only effects.

## Separate benchmark families

Do not combine these into one “RNG speed” number:

| Family | Suggested metric and controls |
|---|---|
| `next_u64` | nanoseconds per word and words/second; fixed seed; checksum outputs |
| `fill` | bytes/second across request sizes; state whether tails are partial |
| `uniform_u64` | time and source draws/result for representative and worst-case-like bounds |
| `secure_bytes` | latency and throughput by request size; identify native backend |
| `secure_uniform_u64` | time and provider draws/result by bound |
| primality | latency by input class and bit length |
| random prime | latency distribution, interval, attempt budget, successes/failures |

For bounded functions, include `b = 1`, powers of two, non-powers of two, and
bounds with large rejection thresholds. For OS calls, small-request latency
and large-buffer throughput answer different engineering questions.

## Optimization constraints

Performance work must preserve:

- canonical 200-fractional-digit inputs;
- exact Euclidean remainders;
- versioned known-answer output and residue vectors;
- unsigned overflow semantics and portable shift counts;
- error propagation and unbiased range mapping;
- GCC/Clang/MSVC builds and C/C++ headers.

Replacing exact limb division with `double` or platform-dependent
`long double` would measure a different algorithm and break the educational
and cross-platform contract. A faster alternative may be valuable, but it
requires an explicitly versioned stream change and matching documentation.
