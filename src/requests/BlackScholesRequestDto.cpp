#include "requests/BlackScholesRequestDto.h"
#include <stdexcept>

namespace dto {

BlackScholesRequestDto::BlackScholesRequestDto(const Json::Value& json) 
    : is_valid_(false), validation_error_("") {
    
    std::string error;
    if (validateRequiredFields(json, error) && validateOptionTypeSpecificFields(json, error)) {
        is_valid_ = true;
    } else {
        validation_error_ = error;
    }
}

bool BlackScholesRequestDto::isValid() const {
    return is_valid_;
}

std::string BlackScholesRequestDto::getValidationError() const {
    return validation_error_;
}

std::optional<BlackScholesRequestDto> BlackScholesRequestDto::fromJson(const Json::Value& json, std::string& error) {
    BlackScholesRequestDto dto(json);
    if (dto.isValid()) {
        return dto;
    } else {
        error = dto.getValidationError();
        return std::nullopt;
    }
}

bool BlackScholesRequestDto::validateRequiredFields(const Json::Value& json, std::string& error) {
    if (!validatePositiveDouble(json, "stock_price", stock_price_, error)) {
        return false;
    }
    
    if (!validatePositiveDouble(json, "strike_price", strike_price_, error)) {
        return false;
    }
    
    if (!validatePositiveDouble(json, "volatility", volatility_, error)) {
        return false;
    }
    
    if (!validateNumericField(json, "risk_free_rate", risk_free_rate_, error)) {
        return false;
    }
    
    std::string option_type_str;
    if (!validateStringField(json, "type", option_type_str, error)) {
        return false;
    }
    
    option_type_ = parseOptionType(option_type_str, error);
    if (!error.empty()) {
        return false;
    }
    
    return true;
}

bool BlackScholesRequestDto::validateOptionTypeSpecificFields(const Json::Value& json, std::string& error) {
    switch (option_type_) {
        case OptionType::REGULAR:
        case OptionType::BINARY:
            if (!validatePositiveDouble(json, "time_to_maturity", time_to_maturity_.emplace(), error)) {
                return false;
            }
            break;
            
        case OptionType::RANDOM_EXPIRATION_CALL:
        case OptionType::RANDOM_EXPIRATION_BINARY_CALL:
            if (!validatePositiveDouble(json, "holding_period", holding_period_.emplace(), error)) {
                return false;
            }
            
            if (json.isMember("volatility_around_holding_period")) {
                if (!validatePositiveDouble(json, "volatility_around_holding_period", 
                                          volatility_around_holding_period_.emplace(), error)) {
                    return false;
                }
            } else {
                volatility_around_holding_period_ = holding_period_;
            }
            break;
    }
    
    return true;
}

bool BlackScholesRequestDto::validatePositiveDouble(const Json::Value& json, const std::string& field, 
                                                   double& value, std::string& error) {
    if (!json.isMember(field)) {
        error = "Missing required field: " + field;
        return false;
    }
    
    if (!json[field].isNumeric()) {
        error = "Field " + field + " must be numeric";
        return false;
    }
    
    value = json[field].asDouble();
    if (value <= 0) {
        error = "Field " + field + " must be positive";
        return false;
    }
    
    return true;
}

bool BlackScholesRequestDto::validateNumericField(const Json::Value& json, const std::string& field, 
                                                 double& value, std::string& error) {
    if (!json.isMember(field)) {
        error = "Missing required field: " + field;
        return false;
    }
    
    if (!json[field].isNumeric()) {
        error = "Field " + field + " must be numeric";
        return false;
    }
    
    value = json[field].asDouble();
    return true;
}

bool BlackScholesRequestDto::validateStringField(const Json::Value& json, const std::string& field, 
                                                std::string& value, std::string& error) {
    if (!json.isMember(field)) {
        error = "Missing required field: " + field;
        return false;
    }
    
    if (!json[field].isString()) {
        error = "Field " + field + " must be a string";
        return false;
    }
    
    value = json[field].asString();
    return true;
}

OptionType BlackScholesRequestDto::parseOptionType(const std::string& type_str, std::string& error) {
    if (type_str == "regular") {
        return OptionType::REGULAR;
    } else if (type_str == "binary") {
        return OptionType::BINARY;
    } else if (type_str == "randomExpirationCall") {
        return OptionType::RANDOM_EXPIRATION_CALL;
    } else if (type_str == "randomExpirationBinaryCall") {
        return OptionType::RANDOM_EXPIRATION_BINARY_CALL;
    } else {
        error = "Field type must be either 'regular', 'binary', 'randomExpirationCall', or 'randomExpirationBinaryCall'";
        return OptionType::REGULAR;
    }
}

} // namespace dto
