@echo off
echo ====================================================
echo Building Firewall Rule Engine in C++17...
echo ====================================================

g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/Packet.cpp src/RuleCondition.cpp src/FirewallRule.cpp src/RuleManager.cpp src/Statistics.cpp src/AuditLogger.cpp src/FirewallEngine.cpp src/ConfigurationManager.cpp src/PacketGenerator.cpp src/SimulationRunner.cpp main.cpp -o firewall.exe

if %ERRORLEVEL% EQU 0 (
    if not exist "build\Debug" mkdir "build\Debug"
    copy /y "firewall.exe" "build\Debug\outDebug.exe" >nul
    copy /y "firewall.exe" "build\Debug\outDebug" >nul
    echo.
    echo BUILD SUCCESSFUL! 'firewall.exe' generated.
) else (
    echo.
    echo BUILD FAILED! Check compiler errors above.
    exit /b 1
)

echo.
echo Building Unit Test Suite 'test_engine.exe'...
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/Packet.cpp src/RuleCondition.cpp src/FirewallRule.cpp src/RuleManager.cpp src/Statistics.cpp src/AuditLogger.cpp src/FirewallEngine.cpp src/ConfigurationManager.cpp tests/test_engine.cpp -o test_engine.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo TEST SUITE COMPILED! 'test_engine.exe' generated.
    echo.
    echo Run '.\firewall.exe' to launch interactive firewall.
    echo Run '.\test_engine.exe' to run the automated unit test suite.
) else (
    echo.
    echo TEST SUITE BUILD FAILED!
)
