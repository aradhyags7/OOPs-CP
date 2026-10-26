@echo off
setlocal
cd /d "%~dp0\.."

echo ======================================================================
echo   Compiling Academic PDFs for Firewall Rule Engine Documentation...
echo ======================================================================

python pdf\generate_pdfs.py

echo.
echo PDF generation complete. Check the 'pdf' folder.
pause
