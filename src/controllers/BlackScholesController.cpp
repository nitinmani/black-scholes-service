#include "controllers/BlackScholesController.h"
#include "utils/ControllerUtils.h"
#include "requests/BlackScholesRequestDto.h"
#include <stdexcept>

#ifdef TEST_MODE
namespace TestBlackScholesService {
    extern CallOption calculateRegularCall(double stock_price, double strike_price, 
                                         double time_to_maturity, double volatility, 
                                         double risk_free_rate);
    extern CallOption calculateBinaryCall(double stock_price, double strike_price, 
                                        double time_to_maturity, double volatility, 
                                        double risk_free_rate);
    extern RandomExpirationCallOption calculateRandomExpirationCall(double stock_price, double strike_price, 
                                                                   double volatility, double risk_free_rate, 
                                                                   double holding_period, double volatility_around_holding_period);
    extern RandomExpirationCallOption calculateRandomExpirationBinaryCall(double stock_price, double strike_price, 
                                                                        double volatility, double risk_free_rate, 
                                                                        double holding_period, double volatility_around_holding_period);
}
#endif

void BlackScholesController::calculate(const HttpRequestPtr& req, 
                                       std::function<void(const HttpResponsePtr&)>&& callback) {
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    
    try {
        Json::Value body;
        Json::Reader reader;
        std::string requestBody(req->getBody());
        if (!reader.parse(requestBody, body)) {
            auto errorResponse = ControllerUtils::createErrorResponse("Invalid JSON format", 400);
            resp->setStatusCode(k400BadRequest);
            resp->setBody(errorResponse.toStyledString());
            callback(resp);
            return;
        }
        
        std::string error;
        auto dto = dto::BlackScholesRequestDto::fromJson(body, error);
        if (!dto) {
            auto errorResponse = ControllerUtils::createErrorResponse(error, 400);
            resp->setStatusCode(k400BadRequest);
            resp->setBody(errorResponse.toStyledString());
            callback(resp);
            return;
        }
        
        switch (dto->getOptionType()) {
            case dto::OptionType::RANDOM_EXPIRATION_CALL: {
#ifdef TEST_MODE
                RandomExpirationCallOption result = TestBlackScholesService::calculateRandomExpirationCall(
                    dto->getStockPrice(), dto->getStrikePrice(), dto->getVolatility(), dto->getRiskFreeRate(),
                    dto->getHoldingPeriod().value(), dto->getVolatilityAroundHoldingPeriod().value());
#else
                RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationCall(
                    dto->getStockPrice(), dto->getStrikePrice(), dto->getVolatility(), dto->getRiskFreeRate(),
                    dto->getHoldingPeriod().value(), dto->getVolatilityAroundHoldingPeriod().value());
#endif
                Json::Value data;
                data["type"] = result.type;
                data["value"] = result.value;
                data["holding_period"] = result.holding_period;
                data["volatility_around_holding_period"] = result.volatility_around_holding_period;
                auto response = ControllerUtils::createSuccessResponse(data);
                resp->setBody(response.toStyledString());
                callback(resp);
                return;
            }
            
            case dto::OptionType::RANDOM_EXPIRATION_BINARY_CALL: {
#ifdef TEST_MODE
                RandomExpirationCallOption result = TestBlackScholesService::calculateRandomExpirationBinaryCall(
                    dto->getStockPrice(), dto->getStrikePrice(), dto->getVolatility(), dto->getRiskFreeRate(),
                    dto->getHoldingPeriod().value(), dto->getVolatilityAroundHoldingPeriod().value());
#else
                RandomExpirationCallOption result = BlackScholesService::calculateRandomExpirationBinaryCall(
                    dto->getStockPrice(), dto->getStrikePrice(), dto->getVolatility(), dto->getRiskFreeRate(),
                    dto->getHoldingPeriod().value(), dto->getVolatilityAroundHoldingPeriod().value());
#endif
                Json::Value data;
                data["type"] = result.type;
                data["value"] = result.value;
                data["holding_period"] = result.holding_period;
                data["volatility_around_holding_period"] = result.volatility_around_holding_period;
                auto response = ControllerUtils::createSuccessResponse(data);
                resp->setBody(response.toStyledString());
                callback(resp);
                return;
            }
            
            case dto::OptionType::BINARY:
            case dto::OptionType::REGULAR: {
                CallOption result;
                if (dto->getOptionType() == dto::OptionType::BINARY) {
#ifdef TEST_MODE
                    result = TestBlackScholesService::calculateBinaryCall(dto->getStockPrice(), dto->getStrikePrice(),
                                                                   dto->getTimeToMaturity().value(), dto->getVolatility(), dto->getRiskFreeRate());
#else
                    result = BlackScholesService::calculateBinaryCall(dto->getStockPrice(), dto->getStrikePrice(),
                                                                   dto->getTimeToMaturity().value(), dto->getVolatility(), dto->getRiskFreeRate());
#endif
                } else {
#ifdef TEST_MODE
                    result = TestBlackScholesService::calculateRegularCall(dto->getStockPrice(), dto->getStrikePrice(),
                                                                    dto->getTimeToMaturity().value(), dto->getVolatility(), dto->getRiskFreeRate());
#else
                    result = BlackScholesService::calculateRegularCall(dto->getStockPrice(), dto->getStrikePrice(),
                                                                    dto->getTimeToMaturity().value(), dto->getVolatility(), dto->getRiskFreeRate());
#endif
                }
                Json::Value data;
                data["type"] = result.type;
                data["value"] = result.value;
                auto response = ControllerUtils::createSuccessResponse(data);
                resp->setBody(response.toStyledString());
                callback(resp);
                break;
            }
        }
        
    } catch (const std::exception& e) {
        auto errorResponse = ControllerUtils::createErrorResponse(e.what(), 500);
        resp->setStatusCode(k500InternalServerError);
        resp->setBody(errorResponse.toStyledString());
        callback(resp);
    }
}
