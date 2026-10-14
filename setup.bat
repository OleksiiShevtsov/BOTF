@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Resolve LIB the same way as cmake/BotfLibRoot.cmake:
rem   1) BOTF_LIB_ROOT env
rem   2) <parent>/LIB  (sibling of this BOTF repo — canonical)
rem   3) ../../../LIB  (legacy WorkTraining nesting)

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

if defined BOTF_LIB_ROOT (
  set "LIB_DIR=%BOTF_LIB_ROOT%"
  goto :lib_ready
)

for %%I in ("%SCRIPT_DIR%\..\LIB") do set "LIB_SIBLING=%%~fI"
for %%I in ("%SCRIPT_DIR%\..\..\..\LIB") do set "LIB_LEGACY=%%~fI"

call :lib_looks_ok "%LIB_SIBLING%"
if "!LIB_OK!"=="1" (
  set "LIB_DIR=%LIB_SIBLING%"
  goto :lib_ready
)

call :lib_looks_ok "%LIB_LEGACY%"
if "!LIB_OK!"=="1" (
  set "LIB_DIR=%LIB_LEGACY%"
  goto :lib_ready
)

rem Prefer sibling of this repo
set "LIB_DIR=%LIB_SIBLING%"

:lib_ready
for %%I in ("%LIB_DIR%\..") do set "PARENT_DIR=%%~fI"
set "BUILD_DIR=%PARENT_DIR%\BUILD"
if defined CMAKE_BUILDS_ROOT set "BUILD_DIR=%CMAKE_BUILDS_ROOT%"

rem Pin: matches typical system protobuf 3.21.x; avoids bleeding-edge main + heavy deps.
set "PROTOBUF_TAG=v21.12"
set "PROTOBUF_URL=https://github.com/protocolbuffers/protobuf.git"

echo [botf] LIB directory:   %LIB_DIR%
echo [botf] BUILD directory: %BUILD_DIR%
if not exist "%LIB_DIR%" mkdir "%LIB_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

pushd "%LIB_DIR%"

call :ensure_dep SFML "https://github.com/SFML/SFML.git"
if errorlevel 1 goto :fail

call :ensure_dep googletest "https://github.com/google/googletest.git"
if errorlevel 1 goto :fail

call :ensure_dep spdlog "https://github.com/gabime/spdlog.git"
if errorlevel 1 goto :fail

call :ensure_protobuf
if errorlevel 1 goto :fail

popd
echo [botf] Done.
echo [botf] LIB=%LIB_DIR%
echo [botf] protobuf=%LIB_DIR%\protobuf  (tag %PROTOBUF_TAG%)
echo [botf] spdlog=%LIB_DIR%\spdlog
if defined CMAKE_BUILDS_ROOT (
  echo [botf] CMAKE_BUILDS_ROOT=%CMAKE_BUILDS_ROOT%
  echo [botf] Use preset: cmake --preset debug
) else (
  echo [botf] WARNING: CMAKE_BUILDS_ROOT is not set — set it to your BUILD folder.
)
endlocal
exit /b 0

:fail
popd 2>nul
echo [botf] Setup failed. Check that LIB path above is your real LIB folder.
endlocal
exit /b 1

:lib_looks_ok
set "LIB_OK=0"
if exist "%~1\SFML\" set "LIB_OK=1"
if exist "%~1\sfml\" set "LIB_OK=1"
if exist "%~1\googletest\" set "LIB_OK=1"
if exist "%~1\protobuf\" set "LIB_OK=1"
if exist "%~1\spdlog\" set "LIB_OK=1"
if exist "%~1\README.md" set "LIB_OK=1"
exit /b 0

:ensure_dep
set "DEP_NAME=%~1"
set "DEP_URL=%~2"
if exist "%DEP_NAME%\" (
  echo [botf] %DEP_NAME% already present in %LIB_DIR% — skip clone
  exit /b 0
)
rem Accept alternate folder names for SFML (e.g. SFML-2.6.1)
if /I "%DEP_NAME%"=="SFML" (
  for /d %%D in (SFML-*) do (
    echo [botf] Found %%D — treat as SFML, skip clone
    exit /b 0
  )
  if exist "sfml\" (
    echo [botf] sfml already present — skip clone
    exit /b 0
  )
)
echo [botf] Cloning %DEP_NAME% into %LIB_DIR%\%DEP_NAME% ...
git clone --depth 1 "%DEP_URL%" "%DEP_NAME%"
exit /b %ERRORLEVEL%

:ensure_protobuf
rem Incomplete tree (interrupted clone) → remove and re-clone.
if exist "protobuf\" if not exist "protobuf\CMakeLists.txt" (
  echo [botf] protobuf looks incomplete — removing and re-cloning
  rmdir /s /q "protobuf" 2>nul
)

if exist "protobuf\CMakeLists.txt" (
  echo [botf] protobuf already present in %LIB_DIR%\protobuf — skip clone
) else (
  if exist "protobuf\" (
    echo [botf] protobuf folder unusable — removing
    rmdir /s /q "protobuf" 2>nul
  )
  echo [botf] Cloning protobuf %PROTOBUF_TAG% into %LIB_DIR%\protobuf ...
  git clone --depth 1 --branch "%PROTOBUF_TAG%" "%PROTOBUF_URL%" protobuf
  if errorlevel 1 exit /b 1
)

rem Submodules: run when missing (not only on first clone).
if not exist "protobuf\third_party\googletest\CMakeLists.txt" (
  echo [botf] Initializing protobuf submodules...
  pushd protobuf
  git submodule update --init --recursive
  if errorlevel 1 (
    popd
    exit /b 1
  )
  popd
) else (
  echo [botf] protobuf submodules look present — skip submodule update
)

if not exist "protobuf\CMakeLists.txt" (
  echo [botf] ERROR: protobuf CMakeLists.txt missing after setup
  exit /b 1
)
echo [botf] protobuf ready: %LIB_DIR%\protobuf
exit /b 0
