# CLI Usage

The CLI target is `trueskill_cli`. It reads JSON from a file, a JSON command
argument, or stdin.

```sh
./build/trueskill_cli <command> [input.json|-|json]
```

Commands:

- `rate`
- `rate-1vs1`
- `quality`
- `quality-1vs1`
- `expose`

## `rate-1vs1`

Input:

```json
{
  "first_player": {"mu": 25.0, "sigma": 8.333333333333},
  "second_player": {"mu": 25.0, "sigma": 8.333333333333},
  "drawn": false
}
```

Command:

```sh
./build/trueskill_cli rate-1vs1 input.json
```

Output:

```json
{
  "first_player": {"mu": 29.395831693, "sigma": 7.17147580701},
  "second_player": {"mu": 20.604168307, "sigma": 7.17147580701}
}
```

Without `"drawn": true`, the first player is treated as the winner.

## `rate`

Input:

```json
{
  "rating_groups": [
    [{"mu": 32.0, "sigma": 7.0}],
    [{"mu": 25.0, "sigma": 8.333333333333}, {"mu": 27.0, "sigma": 6.0}],
    [{"mu": 20.0, "sigma": 8.0}]
  ],
  "ranks": [1, 0, 2],
  "weights": [
    [1.0],
    [1.0, 0.5],
    [1.0]
  ]
}
```

Command:

```sh
./build/trueskill_cli rate input.json
```

Output:

```json
{
  "rating_groups": [
    [{"mu": 31.2792902195, "sigma": 5.98723455705}],
    [{"mu": 28.3919181613, "sigma": 7.19292639796}, {"mu": 27.8792668566, "sigma": 5.90056509681}],
    [{"mu": 17.8152926491, "sigma": 7.08576695558}]
  ]
}
```

Lower rank is better. Equal ranks are draws. Output keeps the same team/player
ordering as input.

Weights are optional. If present, they must mirror the `rating_groups` shape.
Weight `0.0` means the player did not participate and is returned unchanged.
Each team must have at least one positive weight.

## `quality`

Input:

```json
{
  "rating_groups": [
    [{"mu": 25.0, "sigma": 8.333333333333}],
    [{"mu": 25.0, "sigma": 8.333333333333}]
  ]
}
```

Output:

```json
{"quality": 0.4472135955}
```

## `quality-1vs1`

Input:

```json
{
  "first_player": {"mu": 25.0, "sigma": 8.333333333333},
  "second_player": {"mu": 25.0, "sigma": 8.333333333333}
}
```

Output:

```json
{"quality": 0.4472135955}
```

## `expose`

Input:

```json
{
  "rating": {"mu": 29.396, "sigma": 7.171}
}
```

Output:

```json
{"exposure": 7.883}
```

## Custom Environment

Every command accepts an optional `environment` object:

```json
{
  "environment": {
    "mu": 25.0,
    "sigma": 8.333333333333,
    "beta": 4.166666666667,
    "tau": 0.083333333333,
    "draw_probability": 0.10
  },
  "rating": {"mu": 29.396, "sigma": 7.171}
}
```
