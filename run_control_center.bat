@echo off
setlocal
cd /d "%~dp0"

echo ======================================================================
echo   Launching Firewall Rule Engine - Control Center Web Dashboard
echo ======================================================================

echo [1/3] Checking C++ engine binary...
if not exist "firewall.exe" (
    echo Building firewall.exe binary...
    call build.bat
)

echo [2/3] Starting Local Python API Adapter (Port 8000)...
start "Firewall Backend API" cmd /k "python -m uvicorn backend.app:app --host 127.0.0.1 --port 8000"

timeout /t 2 /nobreak >nul

echo [3/3] Starting React Vite Frontend (Port 5173)...
start "Firewall Frontend Dashboard" cmd /k "cd frontend && npm run dev"

timeout /t 2 /nobreak >nul

echo.
echo Opening Firewall Control Center in your default web browser...
start http://127.0.0.1:5173/

echo.
echo ======================================================================
echo   Firewall Control Center is running!
echo   Frontend: http://127.0.0.1:5173/
echo   Backend:  http://127.0.0.1:8000/docs (Interactive Swagger Docs)
echo ======================================================================
echo.
pause
