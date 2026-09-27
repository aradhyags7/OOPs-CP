cmake_minimum_required(VERSION 3.12)
project(FirewallRuleEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include_directories(include)

# Common library sources
set(COMMON_SOURCES
    src/Packet.cpp
    src/RuleCondition.cpp
    src/FirewallRule.cpp
    src/RuleManager.cpp
    src/Statistics.cpp
    src/AuditLogger.cpp
    src/FirewallEngine.cpp
    src/ConfigurationManager.cpp
    src/PacketGenerator.cpp
    src/SimulationRunner.cpp
)

# Interactive Firewall Application
add_executable(firewall main.cpp ${COMMON_SOURCES})

# Unit Test Executable
add_executable(test_engine tests/test_engine.cpp
    src/Packet.cpp
    src/RuleCondition.cpp
    src/FirewallRule.cpp
    src/RuleManager.cpp
    src/Statistics.cpp
    src/AuditLogger.cpp
    src/FirewallEngine.cpp
    src/ConfigurationManager.cpp
)
