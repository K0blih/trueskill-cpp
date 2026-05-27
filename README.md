# trueskill-cpp
C++ implementation of the Microsoft TrueSkill rating algorithm

## Status

This repository currently provides the core C++20 library implementation. CLI and
HTTP service frontends are planned as separate targets that will link against the
same library API. A basic JSON CLI is available as `trueskill_cli`.

## Current capabilities

- Create ratings with the standard TrueSkill defaults or custom environment values.
- Update ratings for one-vs-one matches, team matches, free-for-all matches, and draws.
- Use explicit ranks where lower rank means a better result.
- Use partial-play weights for players who contributed less than a full match.
- Use zero weights for players who did not participate; they are returned unchanged.
- Compute match quality for one-vs-one and multi-team matches.
- Compute conservative exposure as `mu - 3 * sigma`.
- Convert between draw probability and draw margin.

## Planned additions

- Install/export rules for package-manager consumption.
- Richer CLI ergonomics, examples, and shell-completion support.
- HTTP service exposing the same library operations.
- Serialization helpers for common request/response formats.
- More golden tests against the Python reference implementation.

## Build and test

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_TESTS=ON
make -j
ctest --output-on-failure
./trueskill_tests
```

## Documentation

- [Usage example](docs/example.md)
- [CLI usage](docs/cli.md)
- [Public API overview](docs/api.md)
- [Source internals](docs/internals.md)
- [Implementation decisions](docs/design.md)
