cmake_minimum_required(VERSION 3.5)
project(black_scholes_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Drogon REQUIRED)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSL REQUIRED gsl)

# Set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${GSL_INCLUDE_DIRS})

# Main application
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/controllers/BlackScholesController.cpp
    src/requests/BlackScholesRequestDto.cpp
    src/services/BlackScholesService.cpp
    src/utils/ControllerUtils.cpp
    src/utils/BlackScholesUtil.cpp
)

target_link_libraries(${PROJECT_NAME}
    Drogon::Drogon
    ${Boost_LIBRARIES}
    ${GSL_LIBRARIES}
)

# Service layer test
add_executable(black_scholes_service_test
    tests/services/BlackScholesServiceTest.cpp
    src/services/BlackScholesService.cpp
    src/utils/BlackScholesUtil.cpp
)

target_link_libraries(black_scholes_service_test
    GTest::GTest
    GTest::Main
    ${Boost_LIBRARIES}
    ${GSL_LIBRARIES}
)

# Controller layer test
add_executable(black_scholes_controller_test
    tests/controllers/BlackScholesControllerTest.cpp
    src/controllers/BlackScholesController.cpp
    src/requests/BlackScholesRequestDto.cpp
    src/services/BlackScholesService.cpp
    src/utils/ControllerUtils.cpp
    src/utils/BlackScholesUtil.cpp
)

target_link_libraries(black_scholes_controller_test
    GTest::GTest
    GTest::Main
    gmock
    gmock_main
    Drogon::Drogon
    ${Boost_LIBRARIES}
    ${GSL_LIBRARIES}
)

target_compile_definitions(black_scholes_controller_test PRIVATE TEST_MODE)

# Util test
add_executable(black_scholes_util_test
    tests/utils/BlackScholesUtilTest.cpp
    src/utils/BlackScholesUtil.cpp
)

target_link_libraries(black_scholes_util_test
    GTest::GTest
    GTest::Main
    ${Boost_LIBRARIES}
    ${GSL_LIBRARIES}
)

# DTO test
add_executable(black_scholes_request_dto_test
    tests/requests/BlackScholesRequestDtoTest.cpp
    src/requests/BlackScholesRequestDto.cpp
)

target_link_libraries(black_scholes_request_dto_test
    GTest::GTest
    GTest::Main
    jsoncpp
)

# Enable testing
enable_testing()
add_test(NAME BlackScholesServiceTest COMMAND black_scholes_service_test)
add_test(NAME BlackScholesControllerTest COMMAND black_scholes_controller_test)
add_test(NAME BlackScholesUtilTest COMMAND black_scholes_util_test)
add_test(NAME BlackScholesRequestDtoTest COMMAND black_scholes_request_dto_test)
