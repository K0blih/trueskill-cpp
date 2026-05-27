# trueskill-cpp
C++ implementation of the Microsoft TrueSkill rating algorithm

## Status

This repository currently provides the core C++20 library, a basic JSON CLI, and
a basic JSON HTTP service.

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
- Richer HTTP service ergonomics, observability, and deployment examples.
- Serialization helpers for common request/response formats.
- More golden tests against the Python reference implementation.

## Build And Test

The core library has no third-party runtime dependency. Building the CLI or HTTP
service fetches `nlohmann/json` and `cpp-httplib` with CMake `FetchContent`.

### Library

Build only the core library:

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_TESTS=OFF -DTRUESKILL_BUILD_CLI=OFF -DTRUESKILL_BUILD_HTTP=OFF
make -j
```

Build the library with tests:

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_TESTS=ON -DTRUESKILL_BUILD_CLI=OFF -DTRUESKILL_BUILD_HTTP=OFF
make -j
ctest --output-on-failure
./trueskill_tests
```

### CLI

Build the CLI with the library and `nlohmann/json`:

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_CLI=ON
make -j
./trueskill_cli --help
```

Run a direct CLI smoke check:

```sh
./trueskill_cli quality '{"rating_groups":[[{"mu":25,"sigma":8.333333333333}],[{"mu":25,"sigma":8.333333333333}]]}'
```

When tests are enabled, CTest also runs `tests/cli_smoke.sh`, which covers all
CLI commands and an error path.

### HTTP Service

Build the HTTP service with the library, `nlohmann/json`, and `cpp-httplib`:

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_HTTP=ON
make -j
./trueskill_http --host 127.0.0.1 --port 8080
```

In another shell:

```sh
curl http://127.0.0.1:8080/health
curl -sS -H 'Content-Type: application/json' \
  -d '{"rating_groups":[[{"mu":25,"sigma":8.333333333333}],[{"mu":25,"sigma":8.333333333333}]]}' \
  http://127.0.0.1:8080/quality
```

Run all enabled tests, including CLI and HTTP smoke tests:

```sh
mkdir -p build
cd build
cmake .. -DTRUESKILL_BUILD_TESTS=ON -DTRUESKILL_BUILD_CLI=ON -DTRUESKILL_BUILD_HTTP=ON
make -j
ctest --output-on-failure
```

The HTTP smoke test starts the service on loopback and checks every route plus
basic error responses.

## Documentation

- [Usage example](docs/example.md)
- [CLI usage](docs/cli.md)
- [HTTP service usage](docs/http.md)
- [Public API overview](docs/api.md)
- [Source internals](docs/internals.md)
- [Implementation decisions](docs/design.md)
