#pragma once

#include <cstddef>
#include <vector>

namespace skill_rating::detail {

struct Gaussian {
    double pi = 0.0;
    double tau = 0.0;

    static Gaussian from_mu_sigma(double mu, double sigma);

    [[nodiscard]] double mu() const;
    [[nodiscard]] double sigma() const;
};

[[nodiscard]] Gaussian operator*(const Gaussian& lhs, const Gaussian& rhs);
[[nodiscard]] Gaussian operator/(const Gaussian& lhs, const Gaussian& rhs);
[[nodiscard]] double gaussian_delta(const Gaussian& lhs, const Gaussian& rhs);

struct Variable {
    Gaussian value;
};

struct SumLayer {
    std::vector<Variable*> variables;
    std::vector<Gaussian> messages;
    std::vector<double> coeffs;
};

double update_prior(Variable& variable, Gaussian& message, const Gaussian& prior);
double update_likelihood_value(
    Variable& value,
    Gaussian& value_message,
    const Variable& mean,
    const Gaussian& mean_message,
    double variance);
double update_likelihood_mean(
    Variable& mean,
    Gaussian& mean_message,
    const Variable& value,
    const Gaussian& value_message,
    double variance);
double update_sum(std::vector<Variable*>& variables, std::vector<Gaussian>& messages, const std::vector<double>& coeffs, std::size_t index);
double update_trunc(Variable& variable, Gaussian& message, double draw_margin, bool draw);

} // namespace skill_rating::detail
