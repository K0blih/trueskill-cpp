# Public API Overview

The public API lives in `include/trueskill/trueskill.hpp`.

## `trueskill::Rating`

`Rating` stores a player's current skill estimate:

- `mu`: estimated skill mean.
- `sigma`: uncertainty around the estimate.

Higher `mu` is better. Lower `sigma` means the system is more certain.

## `trueskill::Environment`

`Environment` stores the algorithm parameters and performs calculations. The
default constructor uses the standard TrueSkill values:

- `mu = 25.0`
- `sigma = 25.0 / 3.0`
- `beta = 25.0 / 6.0`
- `tau = 25.0 / 300.0`
- `draw_probability = 0.10`

Important methods:

- `create_rating()` creates a default rating.
- `create_rating(mu, sigma)` creates a custom rating with validation.
- `rate(...)` updates arbitrary teams.
- `rate_1vs1(...)` is a convenience wrapper for head-to-head matches.
- `quality(...)` estimates how balanced a match is.
- `quality_1vs1(...)` is a convenience wrapper for head-to-head quality.
- `expose(...)` returns `mu - 3 * sigma`.

## Match Results and Output Ordering

`rate_1vs1(first_player, second_player)` treats `first_player` as the winner
and `second_player` as the loser:

```cpp
auto [updated_winner, updated_loser] = env.rate_1vs1(winner, loser);
```

For a draw, pass `true` as the third argument:

```cpp
auto [updated_a, updated_b] = env.rate_1vs1(player_a, player_b, true);
```

The returned pair always matches the input order.

For general matches, `rate(rating_groups, ranks)` uses `ranks` to describe the
result:

```cpp
trueskill::RatingGroups teams = {
    {team_a_player},
    {team_b_player},
    {team_c_player},
};

std::vector<int> ranks = {1, 0, 2};

auto updated = env.rate(teams, ranks);
```

In that example:

- `teams[1]` has rank `0`, so it wins.
- `teams[0]` has rank `1`, so it places second.
- `teams[2]` has rank `2`, so it places third.

Lower rank values are better. Equal rank values mean a draw. If `ranks` is
omitted, ranks default to input order, so `rating_groups[0]` beats
`rating_groups[1]`, and so on.

The returned `RatingGroups` always keeps the same ordering as the input. If a
player was passed as `rating_groups[2][0]`, their updated rating is returned at
`updated[2][0]`.

## Team Data Types

- `Team` is `std::vector<Rating>`.
- `RatingGroups` is `std::vector<Team>`.
- `Weights` is `std::vector<std::vector<double>>`.

Ranks are passed as `std::vector<int>`. Lower rank is better. Equal ranks mean
a draw. Weights mirror the team/player shape of `RatingGroups`.

Weight rules:

- Missing weights default to `1.0` for every player.
- Positive weights participate in the match update.
- A weight of `0.0` means the player did not participate; their returned rating is unchanged.
- Each team must have at least one player with positive weight.

## Draw Helpers

- `calc_draw_margin(draw_probability, player_count, beta)`
- `calc_draw_probability(draw_margin, player_count, beta)`

These are public because CLI and HTTP frontends will likely expose them.
