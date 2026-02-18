# Black-Scholes Option Pricing Service

A C++ microservice for financial option calculations using the Black-Scholes model. Provides REST API endpoints for calculating option values including standard calls, binary calls, and random expiration options.

## Features

- **Regular Call Options**: Standard Black-Scholes call option pricing
- **Binary (Digital) Call Options**: Binary call option pricing
- **Random Expiration Call Options**: Options with random expiration
- **Random Expiration Binary Call Options**: Binary options with random expiration

## Prerequisites

- CMake 3.5+
- C++17 compiler
- [Drogon](https://github.com/drogonframework/drogon) - C++ web framework
- [Boost](https://www.boost.org/) - C++ libraries
- [GSL](https://www.gnu.org/software/gsl/) - GNU Scientific Library
- [GTest](https://github.com/google/googletest) - Testing framework
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) - JSON library

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running

```bash
./black_scholes_service
```

The service listens on `http://0.0.0.0:8080`.

## API

### Calculate Option Value

**POST** `/api/calculate`

Request body (JSON):

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| stock_price | number | Yes | Current stock price |
| strike_price | number | Yes | Option strike price |
| volatility | number | Yes | Annualized volatility (0-1) |
| risk_free_rate | number | Yes | Risk-free rate (decimal) |
| type | string | Yes | `regular`, `binary`, `randomExpirationCall`, or `randomExpirationBinaryCall` |
| time_to_maturity | number | For regular/binary | Time to maturity in years |
| holding_period | number | For random expiration | Expected holding period in years |
| volatility_around_holding_period | number | No | Volatility around holding period (defaults to holding_period) |

### Example: Regular Call

```bash
curl -X POST http://localhost:8080/api/calculate \
  -H "Content-Type: application/json" \
  -d '{
    "stock_price": 100,
    "strike_price": 95,
    "time_to_maturity": 0.25,
    "volatility": 0.2,
    "risk_free_rate": 0.05,
    "type": "regular"
  }'
```

### Example: Random Expiration Call

```bash
curl -X POST http://localhost:8080/api/calculate \
  -H "Content-Type: application/json" \
  -d '{
    "stock_price": 100,
    "strike_price": 100,
    "volatility": 0.9,
    "risk_free_rate": 0.05,
    "type": "randomExpirationCall",
    "holding_period": 5.0,
    "volatility_around_holding_period": 5.0
  }'
```

## Running Tests

```bash
cd build
./black_scholes_service_test
./black_scholes_controller_test
./black_scholes_util_test
./black_scholes_request_dto_test
```

Or use CTest:

```bash
cd build
ctest
```

## Project Structure

```
├── include/
│   ├── controllers/BlackScholesController.h
│   ├── requests/BlackScholesRequestDto.h
│   ├── services/BlackScholesService.h
│   └── utils/
│       ├── BlackScholesUtil.h
│       └── ControllerUtils.h
├── src/
│   ├── main.cpp
│   ├── controllers/BlackScholesController.cpp
│   ├── requests/BlackScholesRequestDto.cpp
│   ├── services/BlackScholesService.cpp
│   └── utils/
│       ├── BlackScholesUtil.cpp
│       └── ControllerUtils.cpp
└── tests/
    ├── controllers/BlackScholesControllerTest.cpp
    ├── services/BlackScholesServiceTest.cpp
    ├── utils/BlackScholesUtilTest.cpp
    └── requests/BlackScholesRequestDtoTest.cpp
```

## License

MIT
