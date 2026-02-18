#include <gtest/gtest.h>
#include "utils/BlackScholesUtil.h"
#include <cmath>
#include <chrono>
#include <iostream>

class BlackScholesUtilTest : public ::testing::Test {
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

// Test standard Black-Scholes call option calculation
TEST_F(BlackScholesUtilTest, StandardCallOptionTest) {
    double result = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, 
                                                           time_to_maturity, volatility, risk_free_rate);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, stock_price); // Option value should be less than stock price
    
    // Expected value should be around 7.71 for these parameters
    EXPECT_NEAR(result, 7.714, 0.1);
}

// Test binary Black-Scholes call option calculation
TEST_F(BlackScholesUtilTest, BinaryCallOptionTest) {
    double result = BlackScholesUtil::calculateBinaryCall(stock_price, strike_price, 
                                                         time_to_maturity, volatility, risk_free_rate);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0); // Binary option value should be between 0 and 1
    
    // Expected value should be around 0.713 for these parameters
    EXPECT_NEAR(result, 0.713, 0.01);
}

// Test random expiration binary call option calculation with user's sample data
TEST_F(BlackScholesUtilTest, RandomExpirationBinaryCall_UserSample1) {
    // stock price: 100, strike: 100, volatility: 10%, risk free rate 4.22%, holding period 5 years, volatility around holding period 10 years --> $0.55
    double result = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 100.0, 0.1, 0.0422, 5.0, 10.0);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0); // Binary option value should be between 0 and 1
    EXPECT_NEAR(result, 0.55, 0.1); // User's expected value
}

TEST_F(BlackScholesUtilTest, RandomExpirationBinaryCall_UserSample2) {
    // stock price: 100, strike: 100, volatility: 10%, risk free rate 4.22%, holding period 5 years, volatility around holding period 5 years --> $0.61
    double result = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 100.0, 0.1, 0.0422, 5.0, 5.0);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
    EXPECT_NEAR(result, 0.61, 0.1); // User's expected value
}

TEST_F(BlackScholesUtilTest, RandomExpirationBinaryCall_UserSample3) {
    // stock price: 100, strike: 100, volatility: 90%, risk free rate 4.22%, holding period 5 years, volatility around holding period 5 years --> $0.21
    double result = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 100.0, 0.9, 0.0422, 5.0, 5.0);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
    EXPECT_NEAR(result, 0.21, 0.1); // User's expected value
}

// Test random expiration binary call with default volatility_around_holding_period
TEST_F(BlackScholesUtilTest, RandomExpirationBinaryCall_DefaultVolatility) {
    // When volatility_around_holding_period equals holding_period, should behave like deterministic case
    double result = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 100.0, 0.2, 0.05, 1.0, 1.0);
    
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
    
    // Should be close to regular binary call with same parameters
    double regular_binary = BlackScholesUtil::calculateBinaryCall(100.0, 100.0, 1.0, 0.2, 0.05);
    EXPECT_NEAR(result, regular_binary, 0.1);
}

// Test edge cases for random expiration binary call
TEST_F(BlackScholesUtilTest, RandomExpirationBinaryCall_EdgeCases) {
    // Zero strike price should return 1.0
    double zero_strike = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 0.0, 0.2, 0.05, 1.0, 1.0);
    EXPECT_NEAR(zero_strike, 1.0, 0.01);
    
    // Zero volatility around holding period should return deterministic case
    double zero_vol = BlackScholesUtil::calculateRandomExpirationBinaryCall(100.0, 100.0, 0.2, 0.05, 1.0, 0.0);
    double deterministic = BlackScholesUtil::calculateBinaryCall(100.0, 100.0, 1.0, 0.2, 0.05);
    EXPECT_NEAR(zero_vol, deterministic, 0.01);
}

// Test edge cases for standard call
TEST_F(BlackScholesUtilTest, StandardCallEdgeCasesTest) {
    // Deep in-the-money call
    double deep_itm = BlackScholesUtil::calculateStandardCall(200.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(deep_itm, 95.0); // Should be close to intrinsic value
    
    // Deep out-of-the-money call
    double deep_otm = BlackScholesUtil::calculateStandardCall(50.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_LT(deep_otm, 1.0); // Should be very small
    
    // At-the-money call
    double atm = BlackScholesUtil::calculateStandardCall(100.0, 100.0, 0.25, 0.2, 0.05);
    EXPECT_GT(atm, 0.0);
    EXPECT_LT(atm, 10.0);
}

// Test parameter sensitivity for standard call
TEST_F(BlackScholesUtilTest, StandardCallSensitivityTest) {
    // Higher volatility should increase option value
    double low_vol = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, 
                                                            time_to_maturity, 0.1, risk_free_rate);
    double high_vol = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, 
                                                             time_to_maturity, 0.3, risk_free_rate);
    EXPECT_GT(high_vol, low_vol);
    
    // Longer time to maturity should increase option value
    double short_time = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, 
                                                               0.1, volatility, risk_free_rate);
    double long_time = BlackScholesUtil::calculateStandardCall(stock_price, strike_price, 
                                                              0.5, volatility, risk_free_rate);
    EXPECT_GT(long_time, short_time);
}

// Test binary option properties
TEST_F(BlackScholesUtilTest, BinaryCallPropertiesTest) {
    // Binary option should always be between 0 and 1
    double binary = BlackScholesUtil::calculateBinaryCall(stock_price, strike_price, 
                                                         time_to_maturity, volatility, risk_free_rate);
    EXPECT_GE(binary, 0.0);
    EXPECT_LE(binary, 1.0);
    
    // Very deep in-the-money binary should approach 1
    double deep_itm_binary = BlackScholesUtil::calculateBinaryCall(200.0, 50.0, 1.0, 0.1, 0.05);
    EXPECT_GT(deep_itm_binary, 0.9);
    
    // Very deep out-of-the-money binary should approach 0
    double deep_otm_binary = BlackScholesUtil::calculateBinaryCall(50.0, 200.0, 0.1, 0.1, 0.05);
    EXPECT_LT(deep_otm_binary, 0.1);
}

// Test zero strike price edge cases
TEST_F(BlackScholesUtilTest, ZeroStrikePriceTest) {
    // Standard call with zero strike should return stock price
    double standard_zero_strike = BlackScholesUtil::calculateStandardCall(100.0, 0.0, 0.25, 0.2, 0.05);
    EXPECT_NEAR(standard_zero_strike, 100.0, 0.01);
    
    // Binary call with zero strike should return 1
    double binary_zero_strike = BlackScholesUtil::calculateBinaryCall(100.0, 0.0, 0.25, 0.2, 0.05);
    EXPECT_NEAR(binary_zero_strike, 1.0, 0.01);
}

// Test case 1: Basic random expiration call with default volatility_around_holding_period
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test1) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period (same as holding_period)
    );
    
    EXPECT_NEAR(result, 60.751793769124738, 0.01);
}

// Test case 2: Random expiration call with different parameters
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test2) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        85.0,   // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period (same as holding_period)
    );
    
    EXPECT_NEAR(result, 63.64180139800721, 0.01);
}

// Test case 3: Random expiration call with high time volatility
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test3) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        10.0    // volatility_around_holding_period
    );
    
    // With GSL integration, we should get much closer to Python results
    EXPECT_NEAR(result, 41.438539, 0.01); // GSL should give better accuracy
}

// Test case 4: Random expiration call with out-of-the-money strike
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test4) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        115.0,  // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period (same as holding_period)
    );
    
    EXPECT_NEAR(result, 57.424716352410968, 0.01);
}

// Test case 5: Random expiration call with zero time volatility (should fall back to standard Black-Scholes)
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test5) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        115.0,  // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        0.0     // volatility_around_holding_period (zero)
    );
    
    EXPECT_NEAR(result, 69.556, 0.01);
}

// Test case 6: Random expiration call with moderate time volatility
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test6) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        115.0,  // strike_price
        0.9,    // volatility
        0.0406, // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    EXPECT_NEAR(result, 57.425, 0.01);
}

// Test case 7: Random expiration call with very high ratio (should fall back to standard Black-Scholes)
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test7) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        0.1     // volatility_around_holding_period (ratio = 50)
    );
    
    // This should fall back to standard Black-Scholes with holding_period as time_to_maturity
    double expected = BlackScholesUtil::calculateStandardCall(100.0, 100.0, 5.0, 0.9, 0.05);
    EXPECT_NEAR(result, expected, 0.01);
}

// Test case 8: Random expiration call with zero strike price
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test8) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        0.0,    // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        5.0     // volatility_around_holding_period
    );
    
    // Should return stock_price when strike_price is 0
    EXPECT_NEAR(result, 100.0, 0.01);
}

// Test case 9: Random expiration call with very small time volatility
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test9) {
    double result = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        5.0,    // holding_period
        0.01    // volatility_around_holding_period (very small)
    );
    
    // This should fall back to standard Black-Scholes due to high ratio
    double expected = BlackScholesUtil::calculateStandardCall(100.0, 100.0, 5.0, 0.9, 0.05);
    EXPECT_NEAR(result, expected, 0.01);
}

// Test case 10: Random expiration call with different holding periods
TEST_F(BlackScholesUtilTest, RandomExpirationCall_Test10) {
    double result1 = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        1.0,    // holding_period
        1.0     // volatility_around_holding_period
    );
    
    double result2 = BlackScholesUtil::calculateRandomExpirationCall(
        100.0,  // stock_price
        100.0,  // strike_price
        0.9,    // volatility
        0.05,   // risk_free_rate
        10.0,   // holding_period
        10.0    // volatility_around_holding_period
    );
    
    // Longer holding period should generally result in higher option value
    EXPECT_GT(result2, result1);
} 