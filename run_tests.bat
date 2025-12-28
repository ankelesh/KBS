@echo off
set UE_PATH="E:\lib\epic\ue\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set PROJECT_PATH="E:\Projects\unreal_projects\KBS\KBS.uproject"

if "%~1"=="" (
    set TEST_FILTER=KBS
) else (
    set TEST_FILTER=%~1
)

echo Running tests: %TEST_FILTER%
echo.

%UE_PATH% %PROJECT_PATH% ^
-ExecCmds="Automation RunTests %TEST_FILTER%" ^
-unattended -nosplash -NullRHI ^
-log ^

echo.