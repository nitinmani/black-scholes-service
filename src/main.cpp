#include <drogon/drogon.h>
#include "controllers/BlackScholesController.h"

int main() {
    drogon::app()
        .addListener("0.0.0.0", 8080)
        .registerController(std::make_shared<BlackScholesController>())
        .run();
}
