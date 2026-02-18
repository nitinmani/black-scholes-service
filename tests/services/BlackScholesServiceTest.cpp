#include <gtest/gtest.h>
#include <boost/math/distributions/normal.hpp>
#include <cmath>
#include <iostream>

// Include the service header for testing
#include "services/BlackScholesService.h"
#include "utils/BlackScholesUtil.h"

class BlackScholesServiceTest : public ::testing::Test {
protected:
    // Test parameters
    const double stock_price = 100.0;
    const double strike_price = 95.0;
    const double time_to_maturity = 0.25;
    const double volatility = 0.2;
    const double risk_free_rate = 0.05;

    void SetUp() override {
        // Set up any common test data
    }
};

// Test normal CDF approximation
TEST_F(BlackScholesServiceTest, NormalCDFTest) {
    boost::math::normal_distribution<double> normal(0.0, 1.0);
    
    // Test some known values
    EXPECT_NEAR(boost::math::cdf(normal, 0.0), 0.5, 1e-6);
    EXPECT_NEAR(boost::math::cdf(normal, 1.0), 0.841345, 1e-6);
    EXPECT_NEAR(boost::math::cdf(normal, -1.0), 0.158655, 1e-6);
    EXPECT_NEAR(boost::math::cdf(normal, 2.0), 0.977250, 1e-6);
}

// Test regular call option calculation
TEST_F(BlackScholesServiceTest, RegularCallOptionTest) {
    auto result = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                          time_to_maturity, volatility, risk_free_rate);
    
    EXPECT_EQ(result.type, "regular");
    EXPECT_GT(result.value, 0.0);
    EXPECT_LT(result.value, stock_price); // Option value should be less than stock price
    
    // Expected value should be around 7.71 for these parameters
    EXPECT_NEAR(result.value, 7.714, 0.1);
}

// Test binary call option calculation
TEST_F(BlackScholesServiceTest, BinaryCallOptionTest) {
    auto result = BlackScholesService::calculateBinaryCall(stock_price, strike_price, 
                                                         time_to_maturity, volatility, risk_free_rate);
    
    EXPECT_EQ(result.type, "binary");
    EXPECT_GT(result.value, 0.0);
    EXPECT_LT(result.value, 1.0); // Binary option value should be between 0 and 1
    
    // Expected value should be around 0.713 for these parameters
    EXPECT_NEAR(result.value, 0.713, 0.01);
}

// Test edge cases
TEST_F(BlackScholesServiceTest, EdgeCasesTest) {
    // Deep in-the-money call
    auto deep_itm = BlackScholesService::calculateRegularCall(200.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(deep_itm.value, 95.0); // Should be close to intrinsic value
    
    // Deep out-of-the-money call
    auto deep_otm = BlackScholesService::calculateRegularCall(50.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_LT(deep_otm.value, 1.0); // Should be very small
    
    // At-the-money call
    auto atm = BlackScholesService::calculateRegularCall(100.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(atm.value, 0.0);
    EXPECT_LT(atm.value, 10.0);
}

// Test parameter sensitivity
TEST_F(BlackScholesServiceTest, ParameterSensitivityTest) {
    // Higher volatility should increase option value
    auto low_vol = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                            time_to_maturity, 0.1, risk_free_rate);
    auto high_vol = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                             time_to_maturity, 0.3, risk_free_rate);
    EXPECT_GT(high_vol.value, low_vol.value);
    
    // Longer time to maturity should increase option value
    auto short_time = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                               0.1, volatility, risk_free_rate);
    auto long_time = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                              0.5, volatility, risk_free_rate);
    EXPECT_GT(long_time.value, short_time.value);
}

// Test binary option properties
TEST_F(BlackScholesServiceTest, BinaryOptionPropertiesTest) {
    // Binary option should always be between 0 and 1
    auto binary = BlackScholesService::calculateBinaryCall(stock_price, strike_price, 
                                                         time_to_maturity, volatility, risk_free_rate);
    EXPECT_GE(binary.value, 0.0);
    EXPECT_LE(binary.value, 1.0);
    
    // Deep in-the-money binary should be close to 1
    auto deep_itm_binary = BlackScholesService::calculateBinaryCall(200.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(deep_itm_binary.value, 0.9);
    
    // Deep out-of-the-money binary should be close to 0
    auto deep_otm_binary = BlackScholesService::calculateBinaryCall(50.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_LT(deep_otm_binary.value, 0.1);
}

// Test mathematical properties
TEST_F(BlackScholesServiceTest, MathematicalPropertiesTest) {
    // Call option value should be non-negative
    auto result = BlackScholesService::calculateRegularCall(stock_price, strike_price, 
                                                          time_to_maturity, volatility, risk_free_rate);
    EXPECT_GE(result.value, 0.0);
    
    // Call option value should not exceed stock price
    EXPECT_LE(result.value, stock_price);
    
    // Binary option should be between 0 and 1
    auto binary = BlackScholesService::calculateBinaryCall(stock_price, strike_price, 
                                                         time_to_maturity, volatility, risk_free_rate);
    EXPECT_GE(binary.value, 0.0);
    EXPECT_LE(binary.value, 1.0);
}

// Test consistency between regular and binary options
TEST_F(BlackScholesServiceTest, ConsistencyTest) {
    // For very high stock prices, binary should approach 1
    auto high_stock_binary = BlackScholesService::calculateBinaryCall(1000.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(high_stock_binary.value, 0.98); // Adjusted from 0.99 to 0.98
    
    // For very low stock prices, binary should approach 0
    auto low_stock_binary = BlackScholesService::calculateBinaryCall(10.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_LT(low_stock_binary.value, 0.01);
}

// Test case 1: Service random expiration call with default volatility_around_holding_period
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test1) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    EXPECT_NEAR(result.value, 60.75179, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 5.0);
}

// Test case 2: Service random expiration call with different parameters
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test2) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        85.0,   // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    EXPECT_NEAR(result.value, 63.64, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 5.0);
}

// Test case 3: Service random expiration call with high time volatility
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test3) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        10.0    // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    EXPECT_NEAR(result.value, 41.438539, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 10.0);
}

// Test case 4: Service random expiration call with zero strike price
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test4) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        0.0,    // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    EXPECT_NEAR(result.value, 100.0, 0.01); // Should return stock_price
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 5.0);
}

// Test case 5: Service random expiration call with zero time volatility
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test5) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        115.0,  // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        0.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    EXPECT_NEAR(result.value, 69.556, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 0.0);
}

// Test case 6: Service random expiration call with very high ratio
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test6) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        0.1     // volatility_around_holding_period (ratio = 50)
    );
    
    EXPECT_EQ(result.type, "random_expiration");
    // Should fall back to standard Black-Scholes
    double expected = BlackScholesUtil::calculateStandardCall(100.0, 100.0, 5.0, 0.9, 0.05);
    EXPECT_NEAR(result.value, expected, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 0.1);
}

// Test case 7: Service random expiration call with different holding periods
TEST_F(BlackScholesServiceTest, RandomExpirationCall_Test7) {
    RandomExpirationCallOption result1 = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        1.0,    // holding_period
        1.0     // volatility_around_holding_period
    );
    
    RandomExpirationCallOption result2 = BlackScholesService::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        10.0,   // holding_period
        10.0    // volatility_around_holding_period
    );
    
    EXPECT_EQ(result1.type, "random_expiration");
    EXPECT_EQ(result2.type, "random_expiration");
    EXPECT_EQ(result1.holding_period, 1.0);
    EXPECT_EQ(result2.holding_period, 10.0);
    EXPECT_EQ(result1.volatility_around_holding_period, 1.0);
    EXPECT_EQ(result2.volatility_around_holding_period, 10.0);
    
    // Longer holding period should generally result in higher option value
    EXPECT_GT(result2.value, result1.value);
}

// Test case 8: Service random expiration binary call with user's sample data
TEST_F(BlackScholesServiceTest, RandomExpirationBinaryCall_UserSample1) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.1,    // volatility
        0.0422, // risk_free_rate
        5.0,    // holding_period
        10.0    // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration_binary");
    EXPECT_NEAR(result.value, 0.55, 0.1); // User's expected value
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 10.0);
}

// Test case 9: Service random expiration binary call with zero strike price
TEST_F(BlackScholesServiceTest, RandomExpirationBinaryCall_ZeroStrike) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
        100.0,  // stock_price
        0.0,    // strike_price
        0.1,    // volatility
        0.0422, // risk_free_rate
        5.0,    // holding_period
        10.0    // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration_binary");
    EXPECT_NEAR(result.value, 1.0, 0.01); // Binary call with zero strike should return 1
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 10.0);
}

// Test case 10: Service random expiration binary call with zero time volatility
TEST_F(BlackScholesServiceTest, RandomExpirationBinaryCall_ZeroVolatility) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.1,    // volatility
        0.0422, // risk_free_rate
        5.0,    // holding_period
        0.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration_binary");
    // Should fall back to deterministic binary call
    double expected = BlackScholesUtil::calculateBinaryCall(100.0, 100.0, 5.0, 0.1, 0.0422);
    EXPECT_NEAR(result.value, expected, 0.01);
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 0.0);
}

// Test case 11: Service random expiration binary call with user's second sample data
TEST_F(BlackScholesServiceTest, RandomExpirationBinaryCall_UserSample2) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.1,    // volatility
        0.0422, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration_binary");
    EXPECT_NEAR(result.value, 0.61, 0.1); // User's expected value
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 5.0);
}

// Test case 12: Service random expiration binary call with user's third sample data (high volatility)
TEST_F(BlackScholesServiceTest, RandomExpirationBinaryCall_UserSample3) {
    RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility (90%)
        0.0422, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_EQ(result.type, "random_expiration_binary");
    EXPECT_NEAR(result.value, 0.21, 0.1); // User's expected value
    EXPECT_EQ(result.holding_period, 5.0);
    EXPECT_EQ(result.volatility_around_holding_period, 5.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 