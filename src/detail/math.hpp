#pragma once

#include <cstddef>
#include <vector>

namespace trueskill::detail {

constexpr double kEpsilon = 1e-12;

void require_finite_positive(double value, const char* name);
void require_probability(double value, const char* name);

[[nodiscard]] double square(double value) noexcept;
[[nodiscard]] double normal_pdf(double x);
[[nodiscard]] double normal_cdf(double x);
[[nodiscard]] double inverse_normal_cdf(double p);

[[nodiscard]] double determinant(std::vector<std::vector<double>> matrix);
[[nodiscard]] std::vector<double> solve(std::vector<std::vector<double>> matrix, std::vector<double> rhs);

} // namespace trueskill::detail
