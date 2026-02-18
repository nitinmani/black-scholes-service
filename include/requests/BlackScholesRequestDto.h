#pragma once

#include <jsoncpp/json/json.h>
#include <string>
#include <optional>

namespace dto {

enum class OptionType {
    REGULAR,
    BINARY,
    RANDOM_EXPIRATION_CALL,
    RANDOM_EXPIRATION_BINARY_CALL
};

class BlackScholesRequestDto {
public:
    // Constructor from JSON
    explicit BlackScholesRequestDto(const Json::Value& json);
    
    // Validation
    bool isValid() const;
    std::string getValidationError() const;
    
    // Getters
    double getStockPrice() const { return stock_price_; }
    double getStrikePrice() const { return strike_price_; }
    std::optional<double> getTimeToMaturity() const { return time_to_maturity_; }
    double getVolatility() const { return volatility_; }
    double getRiskFreeRate() const { return risk_free_rate_; }
    OptionType getOptionType() const { return option_type_; }
    std::optional<double> getHoldingPeriod() const { return holding_period_; }
    std::optional<double> getVolatilityAroundHoldingPeriod() const { return volatility_around_holding_period_; }
    
    // Static factory method
    static std::optional<BlackScholesRequestDto> fromJson(const Json::Value& json, std::string& error);

private:
    // Validation methods
    bool validateRequiredFields(const Json::Value& json, std::string& error);
    bool validateOptionTypeSpecificFields(const Json::Value& json, std::string& error);
    bool validatePositiveDouble(const Json::Value& json, const std::string& field, double& value, std::string& error);
    bool validateNumericField(const Json::Value& json, const std::string& field, double& value, std::string& error);
    bool validateStringField(const Json::Value& json, const std::string& field, std::string& value, std::string& error);
    OptionType parseOptionType(const std::string& type_str, std::string& error);
    
    // Member variables
    double stock_price_;
    double strike_price_;
    std::optional<double> time_to_maturity_;
    double volatility_;
    double risk_free_rate_;
    OptionType option_type_;
    std::optional<double> holding_period_;
    std::optional<double> volatility_around_holding_period_;
    
    // Validation state
    bool is_valid_;
    std::string validation_error_;
};

} // namespace dto
