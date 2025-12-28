@echo off
REM Build script for KBS Unreal Engine project
REM UE Version: 5.6

echo ========================================
echo Building KBS Project
echo ========================================
echo.
echo Target: KBSEditor
echo Configuration: Development
echo Platform: Win64
echo.

"E:\lib\epic\ue\UE_5.6\Engine\Build\BatchFiles\Build.bat" KBSEditor Win64 Development -Project="%~dp0KBS.uproject" -WaitMutex -FromMsBuild

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
) else (
    echo.
    echo ========================================
    echo Build failed with error code %ERRORLEVEL%
    echo ========================================
)

pause
