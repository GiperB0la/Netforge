@echo off
setlocal

echo ========================================
echo Netforge - Linux x64 Release
echo ========================================
echo.

for /f "delims=" %%i in ('wsl wslpath "%~dp0"') do set WSL_DIR=%%i

wsl bash -lc "cd '%WSL_DIR%' && rm -rf release && cmake -S .. -B release -DCMAKE_BUILD_TYPE=Release -DNETFORGE_BUILD_EXAMPLES=ON -DNETFORGE_BUILD_TESTS=OFF && cmake --build release --parallel"

if errorlevel 1 (
    echo.
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.

pause