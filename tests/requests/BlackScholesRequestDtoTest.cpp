#include <gtest/gtest.h>
#include <jsoncpp/json/json.h>
#include "requests/BlackScholesRequestDto.h"

class BlackScholesRequestDtoTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

// Test valid regular call request
TEST_F(BlackScholesRequestDtoTest, ValidRegularCallRequest) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "regular";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getStockPrice(), 100.0);
    EXPECT_EQ(dto->getStrikePrice(), 100.0);
    EXPECT_EQ(dto->getTimeToMaturity().value(), 1.0);
    EXPECT_EQ(dto->getVolatility(), 0.2);
    EXPECT_EQ(dto->getRiskFreeRate(), 0.05);
    EXPECT_EQ(dto->getOptionType(), dto::OptionType::REGULAR);
}

// Test valid binary call request
TEST_F(BlackScholesRequestDtoTest, ValidBinaryCallRequest) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "binary";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getOptionType(), dto::OptionType::BINARY);
}

// Test valid random expiration call request
TEST_F(BlackScholesRequestDtoTest, ValidRandomExpirationCallRequest) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 5.0;
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getOptionType(), dto::OptionType::RANDOM_EXPIRATION_CALL);
    EXPECT_EQ(dto->getHoldingPeriod().value(), 5.0);
    EXPECT_EQ(dto->getVolatilityAroundHoldingPeriod().value(), 5.0);
    EXPECT_FALSE(dto->getTimeToMaturity().has_value());
}

// Test valid random expiration binary call request
TEST_F(BlackScholesRequestDtoTest, ValidRandomExpirationBinaryCallRequest) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.1;
    requestBody["risk_free_rate"] = 0.0422;
    requestBody["type"] = "randomExpirationBinaryCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 10.0;
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getOptionType(), dto::OptionType::RANDOM_EXPIRATION_BINARY_CALL);
    EXPECT_EQ(dto->getHoldingPeriod().value(), 5.0);
    EXPECT_EQ(dto->getVolatilityAroundHoldingPeriod().value(), 10.0);
    EXPECT_FALSE(dto->getTimeToMaturity().has_value());
}

// Test random expiration binary call with default volatility_around_holding_period
TEST_F(BlackScholesRequestDtoTest, RandomExpirationBinaryCallWithDefaultVolatility) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.1;
    requestBody["risk_free_rate"] = 0.0422;
    requestBody["type"] = "randomExpirationBinaryCall";
    requestBody["holding_period"] = 5.0;
    // Note: volatility_around_holding_period is missing
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getHoldingPeriod().value(), 5.0);
    EXPECT_EQ(dto->getVolatilityAroundHoldingPeriod().value(), 5.0); // Should default to holding_period
}

// Test random expiration call with default volatility_around_holding_period
TEST_F(BlackScholesRequestDtoTest, RandomExpirationCallWithDefaultVolatility) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 85.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.0406;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    // Note: volatility_around_holding_period is missing
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_TRUE(dto.has_value());
    EXPECT_EQ(dto->getHoldingPeriod().value(), 5.0);
    EXPECT_EQ(dto->getVolatilityAroundHoldingPeriod().value(), 5.0); // Should default to holding_period
}

// Test missing required fields
TEST_F(BlackScholesRequestDtoTest, MissingRequiredFields) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    // Missing strike_price, volatility, risk_free_rate, type
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test invalid option type
TEST_F(BlackScholesRequestDtoTest, InvalidOptionType) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "invalid";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test negative values
TEST_F(BlackScholesRequestDtoTest, NegativeValues) {
    Json::Value requestBody;
    requestBody["stock_price"] = -100.0; // Negative
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "regular";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test non-numeric values
TEST_F(BlackScholesRequestDtoTest, NonNumericValues) {
    Json::Value requestBody;
    requestBody["stock_price"] = "not_a_number";
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "regular";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test missing time_to_maturity for regular call
TEST_F(BlackScholesRequestDtoTest, MissingTimeToMaturityForRegularCall) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    // Missing time_to_maturity
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "regular";
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test missing holding_period for random expiration call
TEST_F(BlackScholesRequestDtoTest, MissingHoldingPeriodForRandomExpirationCall) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "randomExpirationCall";
    // Missing holding_period
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

// Test missing holding_period for random expiration binary call
TEST_F(BlackScholesRequestDtoTest, MissingHoldingPeriodForRandomExpirationBinaryCall) {
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.1;
    requestBody["risk_free_rate"] = 0.0422;
    requestBody["type"] = "randomExpirationBinaryCall";
    // Missing holding_period
    
    std::string error;
    auto dto = dto::BlackScholesRequestDto::fromJson(requestBody, error);
    
    ASSERT_FALSE(dto.has_value());
    EXPECT_FALSE(error.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 