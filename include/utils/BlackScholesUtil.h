#pragma once
#include <vector>

namespace BlackScholesUtil {
    /**
     * Calculate the standard Black-Scholes call option price
     */
    double calculateStandardCall(double stock_price, double strike_price, 
                               double time_to_maturity, double volatility, 
                               double risk_free_rate);

    /**
     * Calculate the binary (digital) call option price using Black-Scholes
     */
    double calculateBinaryCall(double stock_price, double strike_price, 
                             double time_to_maturity, double volatility, 
                             double risk_free_rate);

    /**
     * Calculate the random expiration call option price using Black-Scholes with gamma distribution
     */
    double calculateRandomExpirationCall(double stock_price, double strike_price, 
                                       double volatility, double risk_free_rate, 
                                       double holding_period, double volatility_around_holding_period);

    /**
     * Calculate the random expiration binary call option price using Black-Scholes with gamma distribution
     */
    double calculateRandomExpirationBinaryCall(double stock_price, double strike_price, 
                                             double volatility, double risk_free_rate, 
                                             double holding_period, double volatility_around_holding_period);

    /**
     * Calculate multiple standard Black-Scholes call option prices
     */
    std::vector<double> calculateMultipleStandardCalls(const std::vector<double>& stock_prices,
                                                      const std::vector<double>& strike_prices,
                                                      const std::vector<double>& time_to_maturities,
                                                      const std::vector<double>& volatilities,
                                                      const std::vector<double>& risk_free_rates);

    /**
     * Calculate multiple binary call option prices
     */
    std::vector<double> calculateMultipleBinaryCalls(const std::vector<double>& stock_prices,
                                                    const std::vector<double>& strike_prices,
                                                    const std::vector<double>& time_to_maturities,
                                                    const std::vector<double>& volatilities,
                                                    const std::vector<double>& risk_free_rates);

    /**
     * Calculate multiple random expiration call option prices
     */
    std::vector<double> calculateMultipleRandomExpirationCalls(const std::vector<double>& stock_prices,
                                                              const std::vector<double>& strike_prices,
                                                              const std::vector<double>& volatilities,
                                                              const std::vector<double>& risk_free_rates,
                                                              const std::vector<double>& holding_periods,
                                                              const std::vector<double>& volatility_around_holding_periods);

    /**
     * Calculate multiple random expiration binary call option prices
     */
    std::vector<double> calculateMultipleRandomExpirationBinaryCalls(const std::vector<double>& stock_prices,
                                                                    const std::vector<double>& strike_prices,
                                                                    const std::vector<double>& volatilities,
                                                                    const std::vector<double>& risk_free_rates,
                                                                    const std::vector<double>& holding_periods,
                                                                    const std::vector<double>& volatility_around_holding_periods);
}
