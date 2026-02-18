#include "services/BlackScholesService.h"
#include "utils/BlackScholesUtil.h"

CallOption BlackScholesService::calculateRegularCall(double stock_price, double strike_price, 
                                                    double time_to_maturity, double volatility, 
                                                    double risk_free_rate) {
    CallOption result;
    result.type = "regular";
    result.value = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, time_to_maturity, volatility, risk_free_rate);
    return result;
}

CallOption BlackScholesService::calculateBinaryCall(double stock_price, double strike_price, 
                                                   double time_to_maturity, double volatility, 
                                                   double risk_free_rate) {
    CallOption result;
    result.type = "binary";
    result.value = BlackScholesUtil::calculateBinaryCall(stock_price, strike_price, time_to_maturity, volatility, risk_free_rate);
    return result;
}

RandomExpirationCallOption BlackScholesService::calculateRandomExpirationCall(double stock_price, double strike_price, 
                                                                             double volatility, double risk_free_rate, 
                                                                             double holding_period, double volatility_around_holding_period) {
    RandomExpirationCallOption result;
    result.type = "random_expiration";
    result.value = BlackScholesUtil::calculateRandomExpirationCall(stock_price, strike_price, volatility, risk_free_rate, holding_period, volatility_around_holding_period);
    result.holding_period = holding_period;
    result.volatility_around_holding_period = volatility_around_holding_period;
    return result;
}

RandomExpirationCallOption BlackScholesService::calculateRandomExpirationBinaryCall(double stock_price, double strike_price, 
                                                                                   double volatility, double risk_free_rate, 
                                                                                   double holding_period, double volatility_around_holding_period) {
    RandomExpirationCallOption result;
    result.type = "random_expiration_binary";
    result.value = BlackScholesUtil::calculateRandomExpirationBinaryCall(stock_price, strike_price, volatility, risk_free_rate, holding_period, volatility_around_holding_period);
    result.holding_period = holding_period;
    result.volatility_around_holding_period = volatility_around_holding_period;
    return result;
}
