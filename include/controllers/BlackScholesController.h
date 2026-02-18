#pragma once
#include <drogon/HttpController.h>
#include <jsoncpp/json/json.h>
#include "services/BlackScholesService.h"

using namespace drogon;

class BlackScholesController : public HttpController<BlackScholesController, false> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(BlackScholesController::calculate, "/api/calculate", Post);
    METHOD_LIST_END

    void calculate(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

private:
    // No private methods needed - DTO handles all parameter validation
};
