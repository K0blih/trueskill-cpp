#include "detail/factor_graph.hpp"

#include "detail/math.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace trueskill::detail {

Gaussian Gaussian::from_mu_sigma(double mu, double sigma) {
    require_finite_positive(sigma, "sigma");
    const double variance = square(sigma);
    return {1.0 / variance, mu / variance};
}

double Gaussian::mu() const {
    if (pi == 0.0) {
        return 0.0;
    }
    return tau / pi;
}

double Gaussian::sigma() const {
    if (pi == 0.0) {
        return std::numeric_limits<double>::infinity();
    }
    return std::sqrt(1.0 / pi);
}

Gaussian operator*(const Gaussian& lhs, const Gaussian& rhs) {
    return {lhs.pi + rhs.pi, lhs.tau + rhs.tau};
}

Gaussian operator/(const Gaussian& lhs, const Gaussian& rhs) {
    return {lhs.pi - rhs.pi, lhs.tau - rhs.tau};
}

double gaussian_delta(const Gaussian& lhs, const Gaussian& rhs) {
    if (lhs.pi == 0.0 && rhs.pi == 0.0) {
        return 0.0;
    }
    return std::max(std::abs(lhs.mu() - rhs.mu()), std::abs(lhs.sigma() - rhs.sigma()));
}

namespace {

double set_message(Variable& variable, Gaussian& message, const Gaussian& next_message) {
    const Gaussian old_value = variable.value;
    variable.value = (variable.value / message) * next_message;
    message = next_message;
    return gaussian_delta(old_value, variable.value);
}

double v_win(double diff, double draw_margin) {
    const double denom = normal_cdf(diff - draw_margin);
    if (denom < 2.222758749e-162) {
        return -diff + draw_margin;
    }
    return normal_pdf(diff - draw_margin) / denom;
}

double w_win(double diff, double draw_margin) {
    const double v = v_win(diff, draw_margin);
    return v * (v + diff - draw_margin);
}

double v_draw(double diff, double draw_margin) {
    const double abs_diff = std::abs(diff);
    const double denom = normal_cdf(draw_margin - abs_diff) - normal_cdf(-draw_margin - abs_diff);
    if (denom < 2.222758749e-162) {
        return -diff + (diff < 0.0 ? -draw_margin : draw_margin);
    }
    const double numer = normal_pdf(-draw_margin - abs_diff) - normal_pdf(draw_margin - abs_diff);
    return diff < 0.0 ? -numer / denom : numer / denom;
}

double w_draw(double diff, double draw_margin) {
    const double abs_diff = std::abs(diff);
    const double denom = normal_cdf(draw_margin - abs_diff) - normal_cdf(-draw_margin - abs_diff);
    if (denom < 2.222758749e-162) {
        return 1.0;
    }
    const double v = v_draw(abs_diff, draw_margin);
    return square(v) +
        ((draw_margin - abs_diff) * normal_pdf(draw_margin - abs_diff) -
         (-draw_margin - abs_diff) * normal_pdf(-draw_margin - abs_diff)) /
            denom;
}

} // namespace

double update_prior(Variable& variable, Gaussian& message, const Gaussian& prior) {
    return set_message(variable, message, prior);
}

double update_likelihood_value(
    Variable& value,
    Gaussian& value_message,
    const Variable& mean,
    const Gaussian& mean_message,
    double variance) {
    const Gaussian cavity = mean.value / mean_message;
    const double a = 1.0 / (1.0 + variance * cavity.pi);
    return set_message(value, value_message, {a * cavity.pi, a * cavity.tau});
}

double update_likelihood_mean(
    Variable& mean,
    Gaussian& mean_message,
    const Variable& value,
    const Gaussian& value_message,
    double variance) {
    const Gaussian cavity = value.value / value_message;
    const double a = 1.0 / (1.0 + variance * cavity.pi);
    return set_message(mean, mean_message, {a * cavity.pi, a * cavity.tau});
}

double update_sum(std::vector<Variable*>& variables, std::vector<Gaussian>& messages, const std::vector<double>& coeffs, std::size_t index) {
    double mu = 0.0;
    double variance = 0.0;
    for (std::size_t i = 0; i < variables.size(); ++i) {
        if (i == index) {
            continue;
        }
        const Gaussian cavity = variables[i]->value / messages[i];
        mu += coeffs[i] * cavity.mu();
        variance += square(coeffs[i]) * square(cavity.sigma());
    }

    const double coeff = coeffs[index];
    if (std::abs(coeff) < kEpsilon || variance <= 0.0 || !std::isfinite(variance)) {
        throw std::runtime_error("invalid sum factor variance");
    }
    const double target_mu = -mu / coeff;
    const double target_variance = variance / square(coeff);
    return set_message(*variables[index], messages[index], {1.0 / target_variance, target_mu / target_variance});
}

double update_trunc(Variable& variable, Gaussian& message, double draw_margin, bool draw) {
    const Gaussian cavity = variable.value / message;
    const double sqrt_pi = std::sqrt(cavity.pi);
    const double diff = cavity.tau / sqrt_pi;
    const double margin = draw_margin * sqrt_pi;
    const double v = draw ? v_draw(diff, margin) : v_win(diff, margin);
    const double w = draw ? w_draw(diff, margin) : w_win(diff, margin);
    const double denom = 1.0 - w;
    if (denom <= kEpsilon) {
        throw std::runtime_error("truncated Gaussian update did not converge");
    }

    const Gaussian next_value{cavity.pi / denom, (cavity.tau + sqrt_pi * v) / denom};
    return set_message(variable, message, next_value / cavity);
}

} // namespace trueskill::detail
