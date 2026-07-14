# References and provenance

These sources support named algorithms, language rules, platform interfaces,
and test interpretation. The fixed-point constant-remainder composition is
specific to this project and is specified in
[The deterministic game generator](deterministic-prng.md).

## Citing this project

The canonical author credit follows the same software-citation form used by
the author's other research repositories:

```bibtex
@software{classical_rng2026,
  title = {classical\_rng: High-Precision Deterministic and Cryptographic Randomness in C},
  author = {tsotchke},
  version = {2.0.0},
  year = {2026},
  url = {https://github.com/tsotchke/classical_rng},
  note = {Exact fixed-point pi/e roundoff residues with portable operating-system entropy backends}
}
```

The root [`CITATION.cff`](../CITATION.cff) is the machine-readable authority
for citation metadata.

## Deterministic generation

- David Blackman and Sebastiano Vigna,
  [Scrambled Linear Pseudorandom Number Generators](https://arxiv.org/abs/1805.01407),
  2018. Describes the xoshiro/xoroshiro family and scramblers.
- Blackman and Vigna,
  [xoshiro256** reference implementation](https://prng.di.unimi.it/xoshiro256starstar.c)
  and [generator notes](https://prng.di.unimi.it/). The notes distinguish
  noncryptographic PRNGs from stream ciphers and give the standard 53-bit
  binary64 conversion.
- Guy L. Steele Jr., Doug Lea, and Christine H. Flood,
  [Fast Splittable Pseudorandom Number Generators](https://doi.org/10.1145/2660193.2660195),
  OOPSLA 2014. Source for SplitMix.

## Integer semantics and number theory

- ISO/IEC JTC1/SC22/WG14,
  [N1570: C11 committee draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf).
  Section 6.2.5 specifies modulo arithmetic for unsigned integer types;
  section 6.5 describes undefined behavior for unrepresentable signed results.
- Michael O. Rabin,
  [Probabilistic Algorithm for Testing Primality](https://doi.org/10.1016/0022-314X(80)90084-0),
  Journal of Number Theory 12(1), 1980.
- [Deterministic Miller–Rabin SPRP base records](https://miller-rabin.appspot.com/).
  The record attributes the seven bases used here to Jim Sinclair and reports
  coverage through at least `2^64`. This is an exhaustively established bound,
  not a proof embedded in this repository.

## Operating-system random facilities

- Microsoft,
  [`BCryptGenRandom`](https://learn.microsoft.com/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom).
- Linux man-pages project,
  [`getrandom(2)`](https://man7.org/linux/man-pages/man2/getrandom.2.html) and
  [`random(7)`](https://man7.org/linux/man-pages/man7/random.7.html).
- OpenBSD,
  [`arc4random(3)`](https://man.openbsd.org/arc4random.3).

These sources describe the platform contracts. They do not turn this wrapper
into a separately validated cryptographic module.

## Empirical testing

- Pierre L'Ecuyer and Richard Simard,
  [TestU01: A C Library for Empirical Testing of Random Number Generators](https://doi.org/10.1145/1268776.1268777),
  ACM Transactions on Mathematical Software 33(4), 2007.
- Chris Doty-Humphrey,
  [PractRand](https://pracrand.sourceforge.net/).
- NIST,
  [SP 800-22 Rev. 1a](https://csrc.nist.gov/pubs/sp/800/22/r1/upd1/final),
  *A Statistical Test Suite for Random and Pseudorandom Number Generators for
  Cryptographic Applications*. Statistical evidence must be interpreted with
  a stated hypothesis, sample plan, and security model.

## Constant text

The canonical strings are checked into the implementation and exposed through
`crng_game_rng_constant_decimal`. Their provenance is therefore auditable as
source data. Independent verification should compare the strings with a
trusted arbitrary-precision calculation and then check `C mod d` against the
known-answer divisors in the test suite.
