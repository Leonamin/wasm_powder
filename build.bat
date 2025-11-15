@echo off
REM Wasm Powder Toy Build Script (Windows - Modular)
chcp 65001 >nul

echo Building Wasm Powder Toy (Modular)...

REM Emscripten이 설치되어 있는지 확인
where emcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: Emscripten (emcc) not found!
    echo Please install Emscripten:
    echo   1. Download from https://emscripten.org/docs/getting_started/downloads.html
    echo   2. Run emsdk_env.bat to set up environment
    exit /b 1
)

REM 출력 디렉토리 생성
if not exist web mkdir web

REM C++를 WebAssembly로 컴파일 (모든 모듈 포함)
emcc src\simulation.cpp ^
    src\core\grid.cpp ^
    src\physics\heat_conduction.cpp ^
    src\physics\state_change.cpp ^
    src\physics\forces.cpp ^
    src\physics\movement.cpp ^
    src\materials\special_materials.cpp ^
    -o web\simulation.js ^
    -s WASM=1 ^
    -s EXPORTED_FUNCTIONS="[\"_init\",\"_update\",\"_getRenderBufferPtr\",\"_getParticleArrayPtr\",\"_getParticleSize\",\"_addParticleWrapper\",\"_getWidth\",\"_getHeight\",\"_malloc\",\"_free\"]" ^
    -s EXPORTED_RUNTIME_METHODS="[\"ccall\",\"cwrap\",\"HEAP8\",\"HEAP32\",\"HEAPF32\",\"getValue\",\"setValue\"]" ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s INITIAL_MEMORY=33554432 ^
    -O3 ^
    -std=c++11 ^
    -I src

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Output files:
    echo    - web\simulation.js
    echo    - web\simulation.wasm
    echo.
    echo Modular structure:
    echo    - src\core\          (grid management)
    echo    - src\physics\       (simulation passes)
    echo    - src\materials\     (special materials)
    echo.
    echo To run the project:
    echo    cd web ^&^& python -m http.server 8000
    echo    Then open http://localhost:8000
) else (
    echo Build failed!
    exit /b 1
)
