#pragma once
#include <string>

struct CallOption {
    std::string type;
    double value;
};

struct RandomExpirationCallOption {
    std::string type;
    double value;
    double holding_period;
    double volatility_around_holding_period;
};

class BlackScholesService {
public:
    static CallOption calculateRegularCall(double stock_price, double strike_price, 
                                         double time_to_maturity, double volatility, 
                                         double risk_free_rate);
    static CallOption calculateBinaryCall(double stock_price, double strike_price, 
                                        double time_to_maturity, double volatility, 
                                        double risk_free_rate);
    static RandomExpirationCallOption calculateRandomExpirationCall(double stock_price, double strike_price, 
                                                                  double volatility, double risk_free_rate, 
                                                                  double holding_period, double volatility_around_holding_period);
    static RandomExpirationCallOption calculateRandomExpirationBinaryCall(double stock_price, double strike_price, 
                                                                        double volatility, double risk_free_rate, 
                                                                        double holding_period, double volatility_around_holding_period);
};
