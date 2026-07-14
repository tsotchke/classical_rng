# Contributing

Thank you for improving `classical_rng`. Bug reports with a minimal reproducer,
exact revision, compiler/OS information, build flags, and boundary inputs are
especially useful.

## Development gate

```sh
cmake -S . -B build \
  -DCRNG_BUILD_TESTS=ON \
  -DCRNG_BUILD_EXAMPLES=ON \
  -DCRNG_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

With GCC or Clang, also run:

```sh
make sanitize
```

Changes affecting portability should be tested with the relevant GCC, Clang,
AppleClang, and MSVC configurations. Installation changes must compile the
clean CMake consumer, and public-header changes must preserve the C++ inclusion
test.

## Architectural invariants

- Public functions/types begin with `crng_`; public constants begin with
  `CRNG_`.
- Preserve the deterministic game/high-precision module and native crypto
  module as distinct trust domains.
- Never introduce home-grown entropy collection or a silent security fallback.
- Preserve exact base-`10^9` pi/e division unless an explicit versioned
  algorithm change is proposed.
- Use rejection sampling for bounded uniform values.
- Compute mathematical spans in a type that represents the complete domain.
- Avoid compiler extensions in library code unless every supported compiler
  has an isolated, tested equivalent.
- Maintain no hidden global mutable stream state.

## Correctness and test expectations

Add a regression test before or with each bug fix. Select evidence appropriate
to the claim:

- deterministic algorithm changes require output and roundoff known-answer
  vectors plus a documented compatibility decision;
- arithmetic changes require boundary cases and an argument about intermediate
  ranges or language semantics;
- public APIs require null, invalid-input, output-on-success, and state/entropy
  consumption tests;
- native backend changes should add controlled failure injection where
  feasible, including interrupted, partial, and late-failure behavior;
- portability changes require real target compilation, not only preprocessor
  inspection;
- statistical changes must state hypotheses, sample plans, thresholds, and
  what passing does not establish.

ASan/UBSan, frequency plots, and one successful OS call are useful evidence but
not proofs of unexecuted paths, statistical quality, or security.

## Documentation rigor

Every externally visible change should update the relevant header contract,
[API reference](docs/api-reference.md), [guarantee matrix](docs/guarantees.md),
examples, and migration notes. For technical claims:

1. define notation and units;
2. distinguish exact identities, conditional proofs, empirical observations,
   implementation contracts, and non-claims;
3. give intermediate-width or overflow reasoning for numerical code;
4. cite primary papers, standards, or official platform documentation;
5. avoid claiming that tests prove cryptographic unpredictability;
6. identify version and call-schedule scope for deterministic behavior.

The public term “roundoff” is retained for architectural continuity. Use it
precisely: the recorded integer is the exact Euclidean remainder measuring
fixed-point quotient truncation, distinct from the stored-constant truncation
error.

## Pull requests

Describe:

- the input and environment that failed;
- the intended contract and whether it existed before the change;
- the root cause;
- how the new test fails on the old implementation;
- compatibility, security, performance, and documentation effects;
- any remaining gap or non-claim.

Keep discussion technical and respectful. Direct criticism is welcome; insults
do not improve code or review.
