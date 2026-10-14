@echo off
setlocal EnableExtensions

rem CMAKE_BUILDS_ROOT + presets — same as CrashCourse.

if not defined CMAKE_BUILDS_ROOT (
  echo [botf] CMAKE_BUILDS_ROOT is not set. Set the global env var to your BUILD folder.
  exit /b 1
)

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

set "PRESET=debug"
if /I "%~1"=="release" set "PRESET=release"

echo [botf] CMAKE_BUILDS_ROOT=%CMAKE_BUILDS_ROOT%
echo [botf] preset=%PRESET%

pushd "%SCRIPT_DIR%"
cmake --preset %PRESET%
if errorlevel 1 (
  popd
  exit /b 1
)
cmake --build --preset %PRESET%
set "ERR=%ERRORLEVEL%"
popd
exit /b %ERR%
