#include <skill_rating/skill_rating.hpp>

#include <cmath>
#include <iostream>

int main() {
    const skill_rating::Environment env;
    const auto rating = env.create_rating();
    const auto [winner, loser] = env.rate_1vs1(rating, rating);

    if (winner.mu <= rating.mu || loser.mu >= rating.mu) {
        std::cerr << "unexpected rating update\n";
        return 1;
    }
    if (std::abs(env.quality_1vs1(rating, rating) - 0.4472135955) > 0.001) {
        std::cerr << "unexpected match quality\n";
        return 1;
    }
    return 0;
}
