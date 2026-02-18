#include "utils/ControllerUtils.h"

Json::Value ControllerUtils::createSuccessResponse(const Json::Value& data) {
    Json::Value response;
    response["success"] = true;
    response["data"] = data;
    return response;
}

Json::Value ControllerUtils::createErrorResponse(const std::string& error, int statusCode) {
    Json::Value response;
    response["success"] = false;
    response["error"] = error;
    response["status_code"] = statusCode;
    return response;
}

bool ControllerUtils::validatePositiveDouble(const Json::Value& body, const std::string& field, 
                                           double& value, std::string& error) {
    if (!body.isMember(field)) {
        error = "Missing required field: " + field;
        return false;
    }
    
    if (!body[field].isNumeric()) {
        error = "Field " + field + " must be numeric";
        return false;
    }
    
    value = body[field].asDouble();
    if (value <= 0) {
        error = "Field " + field + " must be positive";
        return false;
    }
    
    return true;
}

bool ControllerUtils::validateRequiredField(const Json::Value& body, const std::string& field, 
                                          std::string& value, std::string& error) {
    if (!body.isMember(field)) {
        error = "Missing required field: " + field;
        return false;
    }
    
    if (!body[field].isString()) {
        error = "Field " + field + " must be a string";
        return false;
    }
    
    value = body[field].asString();
    return true;
}
