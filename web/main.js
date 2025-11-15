// 전역 변수
let wasmModule = null;
let renderData = null;
let particleData = null;
let particleSize = 0;
let selectedType = 2; // 기본값: SAND
let isDrawing = false;
let lastMouseX = 0;
let lastMouseY = 0;
let renderMode = 'type'; // 'type' 또는 'temperature'
let WIDTH = 400;
let HEIGHT = 300;

// 브러시 시스템
let brushMode = 'material'; // 'material', 'heat', 'cool'
let brushSize = 3;
let brushShape = 'circle'; // 'circle', 'square'
const MIN_BRUSH_SIZE = 1;
const MAX_BRUSH_SIZE = 20;

// FPS 측정
let lastFrameTime = performance.now();
let frameCount = 0;
let fps = 0;
let fpsUpdateInterval = 0.1; // 0.1초마다 FPS 업데이트
let fpsAccumulator = 0;

// 입자 타입별 색상 (RGB)
const colors = {
    0: [0, 0, 0],           // EMPTY - 검정
    1: [136, 136, 136],     // WALL - 회색
    2: [240, 230, 140],     // SAND - 카키색
    3: [30, 144, 255],      // WATER - 파랑
    4: [175, 238, 238],     // ICE - 하늘색
    5: [245, 245, 245],     // STEAM - 흰색
    6: [255, 69, 0],        // FIRE - 주황/빨강
    
    // 새로운 물질들
    7: [200, 220, 255],     // OXYGEN - 연한 파랑
    8: [255, 200, 200],     // HYDROGEN - 연한 빨강
    9: [80, 70, 50],        // STEAM_OIL - 어두운 갈색 증기
    10: [139, 69, 19],      // WOOD - 갈색
    11: [192, 192, 192],    // IRON - 은색
    12: [220, 220, 220],    // LITHIUM - 밝은 회색
    13: [200, 200, 210],    // SODIUM - 은백색
    14: [100, 80, 40],      // OIL - 어두운 갈색
    15: [180, 180, 180]     // CO2 - 회색 (무거운 기체)
};

// WebAssembly 모듈 로드 (Emscripten 글루 코드 사용)
function loadWasm() {
    // Emscripten Module 설정
    window.Module = {
        onRuntimeInitialized: function() {
            console.log('Wasm Runtime Initialized');
            console.log('Module keys:', Object.keys(Module));
            wasmModule = Module;
            
            // 초기화
            Module._init();
            
            // 그리드 크기 가져오기
            WIDTH = Module._getWidth();
            HEIGHT = Module._getHeight();
            
            // 렌더 버퍼 포인터 가져오기 (바이트 주소)
            const bufferPtr = Module._getRenderBufferPtr();
            
            // Emscripten의 HEAP32 뷰를 사용하여 데이터 접근
            // bufferPtr을 4로 나누어 Int32 인덱스로 변환
            const int32Index = bufferPtr >> 2;
            renderData = Module.HEAP32.subarray(int32Index, int32Index + WIDTH * HEIGHT);
            
            // Particle 배열 접근 설정
            const particlePtr = Module._getParticleArrayPtr();
            particleSize = Module._getParticleSize();
            particleData = particlePtr;
            
            // UI 초기화
            initUI();
            
            // 로딩 화면 숨기고 앱 표시
            document.getElementById('loading').style.display = 'none';
            document.getElementById('app').style.display = 'block';
            
            // 게임 루프 시작
            gameLoop();
            
            // 연속 그리기 루프 시작
            continuousDrawLoop();
        }
    };
    
    // simulation.js 로드
    const script = document.createElement('script');
    script.src = 'simulation.js';
    script.onerror = () => {
        document.getElementById('loading').innerHTML = '❌ simulation.js 로드 실패';
    };
    document.head.appendChild(script);
}

// UI 초기화
function initUI() {
    const canvas = document.getElementById('particleCanvas');
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    
    // addParticle 함수명 변경 (모듈식 빌드 대응)
    console.log('Available functions:', Object.keys(wasmModule).filter(k => k.startsWith('_')));
    if (wasmModule._addParticleWrapper) {
        console.log('Using _addParticleWrapper');
        wasmModule._addParticle = wasmModule._addParticleWrapper;
    } else if (wasmModule._addParticle) {
        console.log('Using _addParticle (already exists)');
    } else {
        console.error('No addParticle function found!');
    }
    
    // 입자 선택 버튼 이벤트
    document.querySelectorAll('.particle-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.particle-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            selectedType = parseInt(btn.dataset.type);
        });
    });
    
    // Clear 버튼 이벤트
    const clearBtn = document.getElementById('clearBtn');
    if (clearBtn) {
        clearBtn.addEventListener('click', clearGrid);
    }
    
    // 렌더링 모드 버튼 이벤트
    document.querySelectorAll('.mode-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.mode-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            renderMode = btn.dataset.mode;
        });
    });
    
    // 브러시 모드 버튼 이벤트
    document.querySelectorAll('.brush-mode-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.brush-mode-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            brushMode = btn.dataset.brushMode;
        });
    });
    
    // 브러시 크기 조절 (마우스 휠)
    canvas.addEventListener('wheel', (e) => {
        e.preventDefault();
        if (e.deltaY < 0) {
            // 휠 위로 - 브러시 크기 증가
            brushSize = Math.min(brushSize + 1, MAX_BRUSH_SIZE);
        } else {
            // 휠 아래로 - 브러시 크기 감소
            brushSize = Math.max(brushSize - 1, MIN_BRUSH_SIZE);
        }
        updateBrushSizeDisplay();
    });
    
    // 브러시 모양 버튼 이벤트
    document.querySelectorAll('.brush-shape-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.brush-shape-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            brushShape = btn.dataset.shape;
        });
    });
    
    // 마우스 이벤트
    canvas.addEventListener('mousedown', (e) => {
        isDrawing = true;
        updateMousePosition(e);
        addParticleAtMouse(e);
    });
    
    canvas.addEventListener('mouseup', () => {
        isDrawing = false;
    });
    
    canvas.addEventListener('mouseleave', () => {
        isDrawing = false;
    });
    
    canvas.addEventListener('mousemove', (e) => {
        updateMousePosition(e);
    });
    
    // 터치 이벤트 (모바일 지원)
    canvas.addEventListener('touchstart', (e) => {
        e.preventDefault();
        isDrawing = true;
        const touch = e.touches[0];
        const rect = canvas.getBoundingClientRect();
        const x = Math.floor(touch.clientX - rect.left);
        const y = Math.floor(touch.clientY - rect.top);
        addParticleAt(x, y);
    });
    
    canvas.addEventListener('touchmove', (e) => {
        e.preventDefault();
        if (isDrawing) {
            const touch = e.touches[0];
            const rect = canvas.getBoundingClientRect();
            const x = Math.floor(touch.clientX - rect.left);
            const y = Math.floor(touch.clientY - rect.top);
            addParticleAt(x, y);
        }
    });
    
    canvas.addEventListener('touchend', (e) => {
        e.preventDefault();
        isDrawing = false;
    });
}

// 마우스 위치 업데이트
function updateMousePosition(e) {
    const canvas = document.getElementById('particleCanvas');
    const rect = canvas.getBoundingClientRect();
    lastMouseX = Math.floor(e.clientX - rect.left);
    lastMouseY = Math.floor(e.clientY - rect.top);
}

// 마우스 위치에 입자 추가
function addParticleAtMouse(e) {
    updateMousePosition(e);
    addParticleAt(lastMouseX, lastMouseY);
}

// 좌표에 브러시 적용
function addParticleAt(x, y) {
    for (let dy = -brushSize; dy <= brushSize; dy++) {
        for (let dx = -brushSize; dx <= brushSize; dx++) {
            // 브러시 모양에 따라 필터링
            let inBrush = false;
            if (brushShape === 'circle') {
                inBrush = (dx * dx + dy * dy <= brushSize * brushSize);
            } else if (brushShape === 'square') {
                inBrush = true;
            }
            
            if (inBrush) {
                const px = x + dx;
                const py = y + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    applyBrush(px, py);
                }
            }
        }
    }
}

// 브러시 효과 적용
function applyBrush(x, y) {
    if (brushMode === 'material') {
        // 물질 생성
        wasmModule._addParticle(x, y, selectedType);
    } else if (brushMode === 'heat') {
        // 가열 (온도 증가)
        const idx = y * WIDTH + x;
        const offset = particleData + idx * particleSize;
        let temp = Module.HEAPF32[(offset + 4) >> 2];
        temp += 20.0; // 매 프레임 20도 증가
        if (temp > 200.0) temp = 200.0;
        Module.HEAPF32[(offset + 4) >> 2] = temp;
    } else if (brushMode === 'cool') {
        // 냉각 (온도 감소)
        const idx = y * WIDTH + x;
        const offset = particleData + idx * particleSize;
        let temp = Module.HEAPF32[(offset + 4) >> 2];
        temp -= 20.0; // 매 프레임 20도 감소
        if (temp < -50.0) temp = -50.0;
        Module.HEAPF32[(offset + 4) >> 2] = temp;
    }
}

// 브러시 크기 표시 업데이트
function updateBrushSizeDisplay() {
    const display = document.getElementById('brushSizeDisplay');
    if (display) {
        display.textContent = brushSize;
    }
}

// FPS 표시 업데이트
function updateFPSDisplay() {
    const display = document.getElementById('fpsDisplay');
    if (display) {
        display.textContent = fps.toFixed(1);
    }
}

// 그리드 전체 지우기
function clearGrid() {
    wasmModule._init();
}

// 연속 그리기 루프 (마우스 pressed 버그 수정)
function continuousDrawLoop() {
    if (isDrawing) {
        addParticleAt(lastMouseX, lastMouseY);
    }
    requestAnimationFrame(continuousDrawLoop);
}

// 메인 게임 루프
function gameLoop() {
    // FPS 측정
    const currentTime = performance.now();
    const deltaTime = (currentTime - lastFrameTime) / 1000; // 초 단위
    lastFrameTime = currentTime;
    
    frameCount++;
    fpsAccumulator += deltaTime;
    
    // 일정 간격마다 FPS 업데이트
    if (fpsAccumulator >= fpsUpdateInterval) {
        fps = frameCount / fpsAccumulator;
        frameCount = 0;
        fpsAccumulator = 0;
        updateFPSDisplay();
    }
    
    // Wasm 시뮬레이션 업데이트
    wasmModule._update();
    
    // 캔버스 렌더링
    render();
    
    // 다음 프레임
    requestAnimationFrame(gameLoop);
}

// Particle 데이터 읽기 헬퍼 함수
function getParticle(index) {
    const offset = particleData + index * particleSize;
    
    // C++ 구조체 오프셋 (바이트 단위)
    // int type (4 bytes)
    // float temperature (4 bytes)
    // int state (4 bytes)
    // float vx, vy (8 bytes)
    // float latent_heat_storage (4 bytes)
    // int life (4 bytes)
    // bool updated_this_frame (1 byte, but padded to 4)
    
    const type = Module.HEAP32[(offset + 0) >> 2];
    const temperature = Module.HEAPF32[(offset + 4) >> 2];
    const state = Module.HEAP32[(offset + 8) >> 2];
    
    return { type, temperature, state };
}

// 온도를 색상으로 변환 (HSL)
function temperatureToColor(temp) {
    // -20°C ~ 150°C를 0~1로 정규화
    const normalized = (temp + 20) / 170;
    const clamped = Math.max(0, Math.min(1, normalized));
    
    // HSL: 파랑(240) → 초록(120) → 빨강(0)
    const hue = (1 - clamped) * 240;
    
    return hslToRgb(hue, 100, 50);
}

// HSL을 RGB로 변환
function hslToRgb(h, s, l) {
    h = h / 360;
    s = s / 100;
    l = l / 100;
    
    let r, g, b;
    
    if (s === 0) {
        r = g = b = l;
    } else {
        const hue2rgb = (p, q, t) => {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1/6) return p + (q - p) * 6 * t;
            if (t < 1/2) return q;
            if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
            return p;
        };
        
        const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        const p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3);
    }
    
    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

// 렌더링
function render() {
    const canvas = document.getElementById('particleCanvas');
    const ctx = canvas.getContext('2d');
    
    // ImageData를 사용한 빠른 렌더링
    const imageData = ctx.createImageData(WIDTH, HEIGHT);
    const data = imageData.data;
    
    if (renderMode === 'type') {
        // 물질 타입 렌더링
        for (let i = 0; i < renderData.length; i++) {
            const type = renderData[i];
            const color = colors[type] || [255, 0, 255]; // 기본값: 마젠타
            
            const idx = i * 4;
            data[idx] = color[0];     // R
            data[idx + 1] = color[1]; // G
            data[idx + 2] = color[2]; // B
            data[idx + 3] = 255;      // A
        }
    } else if (renderMode === 'temperature') {
        // 온도 렌더링
        for (let i = 0; i < WIDTH * HEIGHT; i++) {
            const particle = getParticle(i);
            const color = temperatureToColor(particle.temperature);
            
            const idx = i * 4;
            data[idx] = color[0];     // R
            data[idx + 1] = color[1]; // G
            data[idx + 2] = color[2]; // B
            data[idx + 3] = 255;      // A
        }
    }
    
    ctx.putImageData(imageData, 0, 0);
}

// 페이지 로드 시 Wasm 로드
window.addEventListener('load', loadWasm);
