#pragma once
#include <jsoncpp/json/json.h>
#include <string>

class ControllerUtils {
public:
    // Common JSON response helpers
    static Json::Value createSuccessResponse(const Json::Value& data);
    static Json::Value createErrorResponse(const std::string& error, int statusCode = 400);
    
    // Common validation helpers
    static bool validatePositiveDouble(const Json::Value& body, const std::string& field, 
                                     double& value, std::string& error);
    static bool validateRequiredField(const Json::Value& body, const std::string& field, 
                                    std::string& value, std::string& error);
};
