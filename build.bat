@echo off
REM Wasm 파우더 토이 빌드 스크립트 (Windows)

echo 🔨 Building Wasm Powder Toy...

REM Emscripten이 설치되어 있는지 확인
where emcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Emscripten (emcc) not found!
    echo Please install Emscripten:
    echo   1. Download from https://emscripten.org/docs/getting_started/downloads.html
    echo   2. Run emsdk_env.bat to set up environment
    exit /b 1
)

REM 출력 디렉토리 생성
if not exist web mkdir web

REM C++를 WebAssembly로 컴파일
emcc src\simulation.cpp ^
    -o web\simulation.js ^
    -s WASM=1 ^
    -s EXPORTED_FUNCTIONS="[\"_init\",\"_update\",\"_getRenderBufferPtr\",\"_addParticle\",\"_getWidth\",\"_getHeight\",\"_malloc\",\"_free\"]" ^
    -s EXPORTED_RUNTIME_METHODS="[\"ccall\",\"cwrap\",\"HEAP8\",\"HEAP32\",\"getValue\",\"setValue\"]" ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s INITIAL_MEMORY=33554432 ^
    -O3 ^
    -std=c++11

if %ERRORLEVEL% EQU 0 (
    echo ✅ Build successful!
    echo 📦 Output files:
    echo    - web\simulation.js
    echo    - web\simulation.wasm
    echo.
    echo 🚀 To run the project:
    echo    cd web ^&^& python -m http.server 8000
    echo    Then open http://localhost:8000
) else (
    echo ❌ Build failed!
    exit /b 1
)
