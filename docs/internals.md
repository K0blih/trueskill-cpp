# Source Internals

The public header is intentionally small. Most implementation details live under
`src/`.

## `src/trueskill.cpp`

This file connects the public API to the algorithm internals.

Responsibilities:

- Validate ratings, teams, ranks, weights, and environment parameters.
- Normalize omitted ranks and weights.
- Skip zero-weight players during graph construction and return them unchanged.
- Build the factor graph for a match.
- Run the iterative rating update.
- Compute match quality.
- Preserve input team ordering in returned results.

## `src/detail/math.*`

This module contains numeric helpers that are not specific to the public API.

Important pieces:

- `normal_pdf`
- `normal_cdf`
- `inverse_normal_cdf`
- `determinant`
- `solve`
- validation helpers such as `require_probability`

The inverse CDF is implemented locally to keep the library dependency-free.

## `src/detail/factor_graph.*`

This module contains the TrueSkill message-passing primitives.

Important pieces:

- `Gaussian`: internal precision/precision-mean representation.
- `Variable`: graph variable holding a Gaussian value.
- `SumLayer`: helper for sum factors such as team performance and performance differences.
- `update_prior`
- `update_likelihood_value`
- `update_likelihood_mean`
- `update_sum`
- `update_trunc`

These types are internal on purpose. Users should work with `Rating` and
`Environment`, not factor graph primitives.

## `src/cli/main.cpp`

This file implements the basic JSON CLI.

Responsibilities:

- Parse small JSON request bodies without third-party dependencies.
- Convert JSON into `Rating`, `RatingGroups`, ranks, weights, and environment values.
- Dispatch commands such as `rate`, `rate-1vs1`, `quality`, `quality-1vs1`, and `expose`.
- Print JSON responses.

## `tests/trueskill_tests.cpp`

The tests use a small custom runner. It prints named pass/fail lines when run
directly:

```sh
./build/trueskill_tests
```

The tests currently cover default values, 1v1 quality, wins, draws, team
matches, ranks, partial-play weights, zero-weight players, custom environments,
draw helper round trips, multi-team draws, and validation errors.
