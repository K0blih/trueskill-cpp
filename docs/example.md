# Usage Example

This example shows the current library API for one-vs-one matches, match
quality, conservative exposure, multi-team matches, explicit ranks, and
partial-play weights.

```cpp
#include <skill_rating/skill_rating.hpp>

#include <iostream>
#include <vector>

void print_rating(const char* name, const skill_rating::Rating& rating) {
    std::cout << name << ": mu=" << rating.mu << ", sigma=" << rating.sigma << '\n';
}

int main() {
    skill_rating::Environment env;

    auto player_a = env.create_rating();
    auto player_b = env.create_rating();

    // Without the draw flag, the first argument is the winner and the second is
    // the loser. The returned pair keeps that same order.
    auto [winner, loser] = env.rate_1vs1(player_a, player_b);
    print_rating("winner", winner);
    print_rating("loser", loser);

    std::cout << "1v1 quality: " << env.quality_1vs1(player_a, player_b) << '\n';
    std::cout << "winner exposure: " << env.expose(winner) << '\n';

    skill_rating::RatingGroups teams = {
        {env.create_rating(32.0, 7.0)},
        {env.create_rating(), env.create_rating(27.0, 6.0)},
        {env.create_rating(20.0, 8.0)},
    };

    // Lower rank is better: teams[1] wins, teams[0] is second, teams[2] is last.
    std::vector<int> ranks = {1, 0, 2};

    // Weights match the same team/player shape. The second player on teams[1]
    // receives half participation credit.
    skill_rating::Weights weights = {
        {1.0},
        {1.0, 0.5},
        {1.0},
    };

    // Output keeps input order, so updated[1][0] belongs to teams[1][0].
    skill_rating::RatingGroups updated = env.rate(teams, ranks, weights);
    print_rating("team 2 player 1", updated[1][0]);
    print_rating("team 2 player 2", updated[1][1]);

    std::cout << "match quality: " << env.quality(teams, weights) << '\n';

    return 0;
}
```

Notes:

- Lower rank values are better results. Rank `0` beats rank `1`.
- Equal ranks represent a draw between teams.
- Weights are optional. If omitted, every player gets weight `1.0`.
- Weight `0.0` means a player did not participate and is returned unchanged.
- `expose(rating)` returns `mu - 3 * sigma`, a conservative sortable score.
