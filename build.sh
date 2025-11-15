#!/bin/bash

# Wasm 파우더 토이 빌드 스크립트 (모듈식 구조)

echo "🔨 Building Wasm Powder Toy (Modular)..."

# Emscripten이 설치되어 있는지 확인
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten (emcc) not found!"
    echo "Please install Emscripten:"
    echo "  macOS: brew install emscripten"
    echo "  Linux: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

# 출력 디렉토리 생성
mkdir -p web

# C++를 WebAssembly로 컴파일 (모든 모듈 포함)
emcc src/simulation.cpp \
    src/core/grid.cpp \
    src/physics/heat_conduction.cpp \
    src/physics/state_change.cpp \
    src/physics/forces.cpp \
    src/physics/movement.cpp \
    src/materials/special_materials.cpp \
    src/chemistry/reaction_system.cpp \
    src/chemistry/reaction_registry.cpp \
    src/chemistry/reactions/combustion.cpp \
    src/chemistry/reactions/water_metal.cpp \
    src/chemistry/reactions/evaporation.cpp \
    -o web/simulation.js \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_init","_update","_getRenderBufferPtr","_getParticleArrayPtr","_getParticleSize","_addParticleWrapper","_getWidth","_getHeight","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAP8","HEAP32","HEAPF32","getValue","setValue"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=33554432 \
    -O3 \
    -std=c++17 \
    -I src

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📦 Output files:"
    echo "   - web/simulation.js"
    echo "   - web/simulation.wasm"
    echo ""
    echo "📁 Modular structure:"
    echo "   - src/core/          (grid management)"
    echo "   - src/physics/       (simulation passes)"
    echo "   - src/materials/     (special materials)"
    echo "   - src/chemistry/     (chemical reactions)"
    echo ""
    echo "🚀 To run the project:"
    echo "   cd web && python3 -m http.server 8000"
    echo "   Then open http://localhost:8000"
else
    echo "❌ Build failed!"
    exit 1
fi
