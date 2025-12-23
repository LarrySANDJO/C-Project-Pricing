cmake_minimum_required(VERSION 3.15)
project(pricer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(pybind11_DIR "C:/Users/Larry/AppData/Local/Programs/Python/Python311/Lib/site-packages/pybind11/share/cmake/pybind11")

# Trouve Python (interpréteur + headers + libs)
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Trouve pybind11
find_package(pybind11 REQUIRED)

# Crée le module Python
pybind11_add_module(pricer
    pybind11.cpp
    OptionPricingService.cpp
    Option.cpp
    MarketData.cpp
    PayoffCall.cpp
    PayoffPut.cpp
    EuropeanBlackScholesPricer.cpp
)

# (Optionnel mais propre)
target_include_directories(pricer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
