@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Format only BOTF project C++ files: *.cpp *.h *.hpp
rem under client, server, net, common\sources, tests.
rem Does NOT touch LIB, .proto, or generated *.pb.cc.

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

where clang-format >nul 2>&1
if errorlevel 1 (
  echo [botf] clang-format not found on PATH.
  echo [botf] Install LLVM or enable "C++ Clang Compiler for Windows" in Visual Studio Installer.
  exit /b 1
)

echo [botf] Formatting with:
clang-format --version

set "COUNT=0"
for %%D in (client server net common\sources tests) do (
  if exist "%SCRIPT_DIR%\%%D\" (
    for /r "%SCRIPT_DIR%\%%D" %%F in (*.cpp *.h *.hpp) do (
      echo   %%F
      clang-format -i "%%F"
      set /a COUNT+=1
    )
  )
)

echo [botf] Formatted !COUNT! project C++ file^(s^).
endlocal
exit /b 0
