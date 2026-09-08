@echo off
setlocal

echo ========================================
echo Netforge - Windows x64 Debug
echo ========================================
echo.

if exist debug (
    rmdir /s /q debug
)

cmake -S .. -B debug ^
    -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DNETFORGE_BUILD_EXAMPLES=ON ^
    -DNETFORGE_BUILD_TESTS=OFF

if errorlevel 1 (
    echo.
    echo CMake configuration failed.
    pause
    exit /b 1
)

cmake --build debug --config Debug --parallel

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