cmake_minimum_required(VERSION 3.15)
project(pricer_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pricer
    main.cpp
    Cli.cpp
    OptionPricingService.cpp
    Option.cpp
    MarketData.cpp
    PayoffCall.cpp
    PayoffPut.cpp
    EuropeanBlackScholesPricer.cpp
)

target_include_directories(pricer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
