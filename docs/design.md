# Implementation Decisions

## Compiled Library Instead Of Header-Only

The project is built as a compiled CMake library. This keeps compile times lower
for consumers and gives us a clean target that later CLI and HTTP binaries can
link against.

Header-only would be simpler for a tiny library, but the factor graph and math
implementation are large enough that hiding them in `.cpp` files is cleaner.

## Public API Separate From Internals

Public API lives in `include/trueskill/trueskill.hpp`. Internal implementation
lives in `src/detail/`.

This keeps users on stable concepts:

- ratings
- teams
- ranks
- weights
- environments

It avoids exposing implementation details like Gaussian precision math and
message passing.

## `include/trueskill/` Folder

The nested include folder lets users write:

```cpp
#include <trueskill/trueskill.hpp>
```

This avoids collisions with other libraries that might also have a file named
`rating.hpp`, `environment.hpp`, or `trueskill.hpp` once installed.

## Dependency-Free Core

The core library currently uses only the C++ standard library.

That is why the normal distribution helpers, inverse CDF, and small matrix
helpers are implemented locally. This makes the first version easy to build and
embed. Later frontends can add their own dependencies without forcing them into
the core algorithm library.

## Factor Graph Style

The rating update follows the classic TrueSkill factor-graph approach instead of
using only closed-form 1v1 formulas.

That choice matters because the library needs to support:

- teams
- more than two teams
- free-for-all matches
- draws
- partial-play weights

A factor graph handles those cases with one general update path.

## Plain Test Runner

The tests do not use a third-party framework yet. The custom runner is enough for
the current project size and keeps setup simple.

If test coverage grows substantially, moving to Catch2, doctest, or GoogleTest
would be reasonable.
