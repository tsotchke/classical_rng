# Bit distribution and test interpretation

The repository contains regression-oriented distribution checks, not a
statistical certification campaign. Their thresholds and exact gating behavior
matter when interpreting a passing build.

## What the tests actually execute

The main deterministic unit test draws 210,000 values into seven bounded
buckets from a fixed seed. Each count must lie within a deterministic 4 percent
gross-deviation threshold from 30,000. This is intentionally wide and catches
broken bounds, severe skew, and stuck output; it is not a calibrated
hypothesis test with a declared false-positive rate.

The two module tests produce visualization JSON:

- the deterministic module records 32,768 raw words;
- the secure module records 4,096 OS-backed words;
- both report 16 buckets selected by the low nibble and the one-count for each
  of 64 bit positions;
- the automated module gate checks only that every low-nibble bucket is
  nonempty.

The visualization exposes more measurements than the pass/fail predicate uses.
A plausible chart is therefore diagnostic evidence, not an additional
automated guarantee.

## Deterministic correctness evidence

The suite also checks properties that a histogram cannot:

- fixed 64-bit known-answer outputs;
- exact pi/e divisors, remainders, and mixed residue;
- constant text length and prefix;
- reproducibility across independent objects;
- deterministic fill equality and null/zero-size contracts;
- invalid, singleton, and complete signed ranges;
- `[0,1)` floating bounds;
- fixed primality classifications, including a Carmichael number and a large
  64-bit prime;
- version-1 compatibility facades;
- C++ inclusion and installed-package consumption.

These are stronger evidence for implementation identity and edge handling than
frequency plots. They still cover only the exercised cases.

## Native-source test limitations

Normal tests confirm that the active OS backend succeeds, returns changing
sample buffers in one run, handles public invalid arguments, seeds a game
object, and supports a fixed one-candidate prime case. The implementation does
not currently inject `EINTR`, partial reads, `ENOSYS`, provider failure, or a
Windows negative status. Those paths have static-review evidence but not
failure-injection evidence.

## What balanced samples do not establish

A balanced histogram does not prove:

- that a deterministic stream is unpredictable;
- that its custom output has the claimed period of its internal transition;
- that different seeds are independent;
- that an OS facility is correctly configured on every deployed host;
- that a cryptographic module is validated;
- that no statistical defect appears at a larger sample size.

The high-precision residues are intentionally reproducible even when their
observed bits appear balanced.

## Designing a serious empirical campaign

Before using TestU01, PractRand, or another suite, specify:

1. the generator version and exact output encoding;
2. seeds, number of streams, and stream-splitting assumptions;
3. sample length and stopping rule chosen before observing results;
4. each null hypothesis and significance threshold;
5. correction for multiple comparisons;
6. treatment of repeated or contradictory runs;
7. retention of raw outputs and tool versions.

External suites can reveal candidate defects. They do not replace transition
analysis, proof of bounded mapping, review of entropy provenance, or a security
argument. See [References](references.md) for primary testing literature.
