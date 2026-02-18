#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

// Define test mode before including the controller

#include "controllers/BlackScholesController.h"

// Mock service class
class MockBlackScholesService {
public:
    MOCK_METHOD(CallOption, calculateRegularCall, (double, double, double, double, double));
    MOCK_METHOD(CallOption, calculateBinaryCall, (double, double, double, double, double));
    MOCK_METHOD(RandomExpirationCallOption, calculateRandomExpirationCall, (double, double, double, double, double, double));
    MOCK_METHOD(RandomExpirationCallOption, calculateRandomExpirationBinaryCall, (double, double, double, double, double, double));
};

// Global mock instance
MockBlackScholesService* g_mockService = nullptr;

// Test-specific service functions that override the real ones
namespace TestBlackScholesService {
    CallOption calculateRegularCall(double stock_price, double strike_price, 
                                   double time_to_maturity, double volatility, 
                                   double risk_free_rate) {
        if (g_mockService) {
            return g_mockService->calculateRegularCall(stock_price, strike_price, 
                                                     time_to_maturity, volatility, risk_free_rate);
        }
        return CallOption{"regular", 0.0};
    }
    
    CallOption calculateBinaryCall(double stock_price, double strike_price, 
                                  double time_to_maturity, double volatility, 
                                  double risk_free_rate) {
        if (g_mockService) {
            return g_mockService->calculateBinaryCall(stock_price, strike_price, 
                                                    time_to_maturity, volatility, risk_free_rate);
        }
        return CallOption{"binary", 0.0};
    }
    
    RandomExpirationCallOption calculateRandomExpirationCall(double stock_price, double strike_price, 
                                                            double volatility, double risk_free_rate, 
                                                            double holding_period, double volatility_around_holding_period) {
        if (g_mockService) {
            return g_mockService->calculateRandomExpirationCall(stock_price, strike_price, 
                                                              volatility, risk_free_rate, 
                                                              holding_period, volatility_around_holding_period);
        }
        return RandomExpirationCallOption{"random_expiration", 0.0, 0.0, 0.0};
    }
    
    RandomExpirationCallOption calculateRandomExpirationBinaryCall(double stock_price, double strike_price, 
                                                                 double volatility, double risk_free_rate, 
                                                                 double holding_period, double volatility_around_holding_period) {
        if (g_mockService) {
            return g_mockService->calculateRandomExpirationBinaryCall(stock_price, strike_price, 
                                                                    volatility, risk_free_rate, 
                                                                    holding_period, volatility_around_holding_period);
        }
        return RandomExpirationCallOption{"random_expiration_binary", 0.0, 0.0, 0.0};
    }
}

class BlackScholesControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_mockService = &mockService;
    }
    
    void TearDown() override {
        g_mockService = nullptr;
    }
    
    MockBlackScholesService mockService;
};

// Test case 1: Controller successfully handles valid random expiration call request
TEST_F(BlackScholesControllerTest, RandomExpirationCall_Success) {
    // Setup mock expectations
    RandomExpirationCallOption expectedResult{"random_expiration", 60.70572, 5.0, 5.0};
    EXPECT_CALL(mockService, calculateRandomExpirationCall(100.0, 100.0, 0.9, 0.05, 5.0, 5.0))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 5.0;
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "random_expiration");
        EXPECT_NEAR(response["data"]["value"].asDouble(), 60.70572, 0.01);
        EXPECT_EQ(response["data"]["holding_period"].asDouble(), 5.0);
        EXPECT_EQ(response["data"]["volatility_around_holding_period"].asDouble(), 5.0);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 2: Controller handles missing volatility_around_holding_period (defaults to holding_period)
TEST_F(BlackScholesControllerTest, RandomExpirationCall_DefaultVolatilityAroundHoldingPeriod) {
    // Setup mock expectations - note that volatility_around_holding_period should default to holding_period
    RandomExpirationCallOption expectedResult{"random_expiration", 63.62, 5.0, 5.0};
    EXPECT_CALL(mockService, calculateRandomExpirationCall(100.0, 85.0, 0.9, 0.0406, 5.0, 5.0))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request without volatility_around_holding_period
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 85.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.0406;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    // Note: volatility_around_holding_period is missing
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "random_expiration");
        EXPECT_NEAR(response["data"]["value"].asDouble(), 63.62, 0.01);
        EXPECT_EQ(response["data"]["holding_period"].asDouble(), 5.0);
        EXPECT_EQ(response["data"]["volatility_around_holding_period"].asDouble(), 5.0); // Should default to holding_period
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 3: Controller successfully handles regular call request
TEST_F(BlackScholesControllerTest, RegularCall_Success) {
    // Setup mock expectations
    CallOption expectedResult{"regular", 25.0};
    EXPECT_CALL(mockService, calculateRegularCall(100.0, 100.0, 1.0, 0.2, 0.05))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "regular";
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "regular");
        EXPECT_EQ(response["data"]["value"].asDouble(), 25.0);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 4: Controller successfully handles binary call request
TEST_F(BlackScholesControllerTest, BinaryCall_Success) {
    // Setup mock expectations
    CallOption expectedResult{"binary", 0.6};
    EXPECT_CALL(mockService, calculateBinaryCall(100.0, 100.0, 1.0, 0.2, 0.05))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "binary";
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "binary");
        EXPECT_EQ(response["data"]["value"].asDouble(), 0.6);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 5: Controller handles invalid JSON format
TEST_F(BlackScholesControllerTest, InvalidJson_ReturnsBadRequest) {
    // Create invalid JSON request
    std::string invalidJson = "{ invalid json }";
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(invalidJson);
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_FALSE(response["success"].asBool());
        EXPECT_TRUE(response["error"].asString().find("Invalid JSON format") != std::string::npos);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 6: Controller handles missing required fields
TEST_F(BlackScholesControllerTest, MissingRequiredFields_ReturnsBadRequest) {
    // Create test request with missing fields
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    // Missing strike_price, volatility, risk_free_rate, type, etc.
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_FALSE(response["success"].asBool());
        EXPECT_TRUE(response["error"].asString().find("Missing required field") != std::string::npos);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 7: Controller handles negative values
TEST_F(BlackScholesControllerTest, NegativeValues_ReturnsBadRequest) {
    // Create test request with negative values
    Json::Value requestBody;
    requestBody["stock_price"] = -100.0;  // Negative stock price
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 5.0;
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_FALSE(response["success"].asBool());
        EXPECT_TRUE(response["error"].asString().find("stock_price") != std::string::npos);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 8: Controller handles invalid option type
TEST_F(BlackScholesControllerTest, InvalidOptionType_ReturnsBadRequest) {
    // Create test request with invalid option type
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["time_to_maturity"] = 1.0;
    requestBody["volatility"] = 0.2;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "invalid_type";  // Invalid type
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_FALSE(response["success"].asBool());
        EXPECT_TRUE(response["error"].asString().find("type must be either") != std::string::npos);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 9: Controller handles service exception
TEST_F(BlackScholesControllerTest, ServiceException_ReturnsInternalServerError) {
    // Setup mock to throw exception
    EXPECT_CALL(mockService, calculateRandomExpirationCall(100.0, 100.0, 0.9, 0.05, 5.0, 5.0))
        .WillOnce(::testing::Throw(std::runtime_error("Service calculation failed")));
    
    // Create test request
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.9;
    requestBody["risk_free_rate"] = 0.05;
    requestBody["type"] = "randomExpirationCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 5.0;
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k500InternalServerError);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_FALSE(response["success"].asBool());
        EXPECT_TRUE(response["error"].asString().find("Service calculation failed") != std::string::npos);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 10: Controller successfully handles random expiration binary call request
TEST_F(BlackScholesControllerTest, RandomExpirationBinaryCall_Success) {
    // Setup mock expectations
    RandomExpirationCallOption expectedResult{"random_expiration_binary", 0.55, 5.0, 10.0};
    EXPECT_CALL(mockService, calculateRandomExpirationBinaryCall(100.0, 100.0, 0.1, 0.0422, 5.0, 10.0))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.1;
    requestBody["risk_free_rate"] = 0.0422;
    requestBody["type"] = "randomExpirationBinaryCall";
    requestBody["holding_period"] = 5.0;
    requestBody["volatility_around_holding_period"] = 10.0;
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "random_expiration_binary");
        EXPECT_NEAR(response["data"]["value"].asDouble(), 0.55, 0.01);
        EXPECT_EQ(response["data"]["holding_period"].asDouble(), 5.0);
        EXPECT_EQ(response["data"]["volatility_around_holding_period"].asDouble(), 10.0);
    });
    
    EXPECT_TRUE(callbackCalled);
}

// Test case 11: Controller handles random expiration binary call with default volatility_around_holding_period
TEST_F(BlackScholesControllerTest, RandomExpirationBinaryCall_DefaultVolatilityAroundHoldingPeriod) {
    // Setup mock expectations - note that volatility_around_holding_period should default to holding_period
    RandomExpirationCallOption expectedResult{"random_expiration_binary", 0.61, 5.0, 5.0};
    EXPECT_CALL(mockService, calculateRandomExpirationBinaryCall(100.0, 100.0, 0.1, 0.0422, 5.0, 5.0))
        .WillOnce(::testing::Return(expectedResult));
    
    // Create test request without volatility_around_holding_period
    Json::Value requestBody;
    requestBody["stock_price"] = 100.0;
    requestBody["strike_price"] = 100.0;
    requestBody["volatility"] = 0.1;
    requestBody["risk_free_rate"] = 0.0422;
    requestBody["type"] = "randomExpirationBinaryCall";
    requestBody["holding_period"] = 5.0;
    // Note: volatility_around_holding_period is missing
    
    // Create HTTP request
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/calculate");
    req->setBody(requestBody.toStyledString());
    
    // Create controller and call method
    BlackScholesController controller;
    bool callbackCalled = false;
    
    controller.calculate(req, [&](const drogon::HttpResponsePtr& resp) {
        callbackCalled = true;
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        
        // Parse response
        Json::Value response;
        Json::Reader reader;
        EXPECT_TRUE(reader.parse(std::string(resp->getBody()), response));
        
        EXPECT_TRUE(response["success"].asBool());
        EXPECT_EQ(response["data"]["type"].asString(), "random_expiration_binary");
        EXPECT_NEAR(response["data"]["value"].asDouble(), 0.61, 0.01);
        EXPECT_EQ(response["data"]["holding_period"].asDouble(), 5.0);
        EXPECT_EQ(response["data"]["volatility_around_holding_period"].asDouble(), 5.0); // Should default to holding_period
    });
    
    EXPECT_TRUE(callbackCalled);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 