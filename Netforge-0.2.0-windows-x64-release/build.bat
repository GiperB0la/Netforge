@echo off
setlocal

echo ========================================
echo Netforge - Windows x64 Release
echo ========================================
echo.

if exist release (
    rmdir /s /q release
)

cmake -S .. -B release ^
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

cmake --build release --config Release --parallel

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