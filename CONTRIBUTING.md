# Contributing to Classical RNG

We love your input! We want to make contributing to Classical RNG as easy and transparent as possible, whether it's:

- Reporting a bug
- Discussing the current state of the code
- Submitting a fix
- Proposing new features
- Becoming a maintainer

## Development Process

All changes happen through pull requests. Pull requests are the best way to propose changes:

1. Fork the repo and create your branch from `main`
2. If you've added code that should be tested, add tests
3. If you've changed APIs, update the documentation
4. Ensure all tests pass
5. Make sure your code follows the existing style
6. Issue your pull request

## Testing

### Running Tests
```bash
# Run all tests
make test

# Run specific test suite
./build/test_game_rng
./build/test_crypto_rng

# Run with visualization
make viz
```

### Adding Tests
1. Add test cases to appropriate test file in `tests/`
2. Update statistical tests if needed in `tests/test_utils/`
3. Add visualization support if relevant

## Code Style

- Use consistent indentation (4 spaces)
- Keep lines under 80 characters
- Comment complex algorithms
- Follow existing naming conventions
- Write clear commit messages

## Bug Reports

Report bugs using GitHub's [issue tracker](../../issues)

**Great Bug Reports** tend to have:

- A quick summary and/or background
- Steps to reproduce
  - Be specific!
  - Give sample code if you can
- What you expected would happen
- What actually happens
- Notes (possibly including why you think this might be happening)

## License
By contributing, you agree that your contributions will be licensed under its MIT License.
