#!/bin/bash
echo "===================================================="
echo "Building Firewall Rule Engine in C++17..."
echo "===================================================="

g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/Packet.cpp src/RuleCondition.cpp src/FirewallRule.cpp src/RuleManager.cpp src/Statistics.cpp src/AuditLogger.cpp src/FirewallEngine.cpp src/ConfigurationManager.cpp src/PacketGenerator.cpp src/SimulationRunner.cpp main.cpp -o firewall

if [ $? -eq 0 ]; then
    mkdir -p build/Debug
    cp firewall build/Debug/outDebug
    echo ""
    echo "BUILD SUCCESSFUL! './firewall' generated."
else
    echo ""
    echo "BUILD FAILED! Check compiler errors above."
    exit 1
fi

echo ""
echo "Building Unit Test Suite 'test_engine'..."
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/Packet.cpp src/RuleCondition.cpp src/FirewallRule.cpp src/RuleManager.cpp src/Statistics.cpp src/AuditLogger.cpp src/FirewallEngine.cpp src/ConfigurationManager.cpp tests/test_engine.cpp -o test_engine

if [ $? -eq 0 ]; then
    echo ""
    echo "TEST SUITE COMPILED! './test_engine' generated."
    echo ""
    echo "Run './firewall' to launch interactive firewall."
    echo "Run './test_engine' to run the automated unit test suite."
else
    echo ""
    echo "TEST SUITE BUILD FAILED!"
fi
