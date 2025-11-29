// 전역 변수
let wasmModule = null;
let renderData = null;
let particleData = null;
let particleSize = 0;
let selectedType = 2; // SAND
let isDrawing = false;
let lastMouseX = 0;
let lastMouseY = 0;
let renderMode = 'type'; // 'type' or 'temperature'

// 고정 그리드 크기 (types.h와 일치해야 함)
const WIDTH = 400;
const HEIGHT = 300;

// 브러시 설정
let brushMode = 'material'; // 'material', 'heat', 'cool'
let brushSize = 3; // 기본값

// FPS
let lastFrameTime = performance.now();
let frameCount = 0;
let fps = 0;

// 색상표
const colors = {
    0: [0, 0, 0],
    1: [136, 136, 136],
    2: [240, 230, 140],
    3: [30, 144, 255],
    4: [175, 238, 238],
    5: [245, 245, 245],
    6: [255, 69, 0],
    7: [200, 220, 255],
    8: [255, 200, 200],
    9: [80, 70, 50],
    10: [139, 69, 19],
    11: [192, 192, 192],
    12: [220, 220, 220],
    13: [200, 200, 210],
    14: [100, 80, 40],
    15: [180, 180, 180]
};

// Wasm 로드
function loadWasm() {
    window.Module = {
        onRuntimeInitialized: function() {
            console.log('Wasm Runtime Initialized');
            wasmModule = Module;
            
            // 초기화 (고정 크기)
            Module._init();
            
            // 데이터 뷰 설정 (한 번만)
            const bufferPtr = Module._getRenderBufferPtr();
            const int32Index = bufferPtr >> 2;
            renderData = Module.HEAP32.subarray(int32Index, int32Index + WIDTH * HEIGHT);
            
            particleData = Module._getParticleArrayPtr();
            particleSize = Module._getParticleSize();
            
            initUI();
            
            document.getElementById('loading').style.display = 'none';
            
            gameLoop();
            continuousDrawLoop();
        }
    };
    
    const script = document.createElement('script');
    script.src = 'simulation.js';
    document.head.appendChild(script);
}

// UI 초기화
function initUI() {
    const canvas = document.getElementById('particleCanvas');
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    
    // 함수 바인딩
    if (wasmModule._addParticleWrapper) {
        wasmModule._addParticle = wasmModule._addParticleWrapper;
    }
    
    // 입자 버튼
    document.querySelectorAll('.particle-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            if (btn.classList.contains('brush-mode-btn')) {
                brushMode = btn.dataset.brushMode;
            } else {
                selectedType = parseInt(btn.dataset.type);
                brushMode = 'material';
            }
            
            // 모든 버튼 비활성화 후 클릭한 것만 활성화
            document.querySelectorAll('.particle-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });
    
    // 뷰 모드 토글
    document.getElementById('modeToggle').addEventListener('click', () => {
        renderMode = (renderMode === 'type') ? 'temperature' : 'type';
        document.getElementById('modeToggle').textContent = 
            (renderMode === 'type') ? '🎨 물질 보기' : '🌡️ 온도 보기';
    });
    
    // 마우스 이벤트
    canvas.addEventListener('mousedown', (e) => { isDrawing = true; addParticleAtMouse(e); });
    canvas.addEventListener('mouseup', () => { isDrawing = false; });
    canvas.addEventListener('mouseleave', () => { isDrawing = false; });
    canvas.addEventListener('mousemove', (e) => { updateMousePosition(e); });
    
    // 휠로 브러시 크기 조절
    canvas.addEventListener('wheel', (e) => {
        e.preventDefault();
        if (e.deltaY < 0) brushSize = Math.min(brushSize + 1, 20);
        else brushSize = Math.max(brushSize - 1, 1);
    });
}

function updateMousePosition(e) {
    const canvas = document.getElementById('particleCanvas');
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    
    lastMouseX = Math.floor((e.clientX - rect.left) * scaleX);
    lastMouseY = Math.floor((e.clientY - rect.top) * scaleY);
    
    // 범위 제한
    if (lastMouseX < 0) lastMouseX = 0;
    if (lastMouseX >= WIDTH) lastMouseX = WIDTH - 1;
    if (lastMouseY < 0) lastMouseY = 0;
    if (lastMouseY >= HEIGHT) lastMouseY = HEIGHT - 1;
}

function addParticleAtMouse(e) {
    updateMousePosition(e);
    addParticleAt(lastMouseX, lastMouseY);
}

function addParticleAt(x, y) {
    for (let dy = -brushSize; dy <= brushSize; dy++) {
        for (let dx = -brushSize; dx <= brushSize; dx++) {
            if (dx*dx + dy*dy <= brushSize*brushSize) {
                const px = x + dx;
                const py = y + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    applyBrush(px, py);
                }
            }
        }
    }
}

function applyBrush(x, y) {
    if (!wasmModule) return;
    
    if (brushMode === 'material') {
        wasmModule._addParticle(x, y, selectedType);
    } else {
        const idx = y * WIDTH + x;
        const offset = particleData + idx * particleSize;
        const tempIdx = (offset + 4) >> 2;
        let temp = wasmModule.HEAPF32[tempIdx];
        
        if (brushMode === 'heat') {
            temp += 20.0;
            if (temp > 200.0) temp = 200.0;
        } else {
            temp -= 20.0;
            if (temp < -50.0) temp = -50.0;
        }
        wasmModule.HEAPF32[tempIdx] = temp;
    }
}

function clearGrid() {
    wasmModule._init();
}

function continuousDrawLoop() {
    if (isDrawing) addParticleAt(lastMouseX, lastMouseY);
    requestAnimationFrame(continuousDrawLoop);
}

function gameLoop() {
    // FPS
    const now = performance.now();
    frameCount++;
    if (now - lastFrameTime >= 1000) {
        document.getElementById('fpsDisplay').textContent = frameCount;
        frameCount = 0;
        lastFrameTime = now;
    }
    
    wasmModule._update();
    render();
    requestAnimationFrame(gameLoop);
}

// 온도 -> 색상 변환
function temperatureToColor(temp) {
    const t = Math.max(0, Math.min(1, (temp + 20) / 170));
    const hue = (1 - t) * 240; // Blue to Red
    return `hsl(${hue}, 100%, 50%)`; // CSS string (느림, but ok for simple) -> ImageData 쓰려면 RGB 변환 필요
}

// HSL to RGB (단순화)
function getTempColor(temp) {
    const t = Math.max(0, Math.min(1, (temp + 20) / 170));
    const hue = (1 - t) * 240;
    // HSL to RGB
    const s = 1, l = 0.5;
    const c = (1 - Math.abs(2 * l - 1)) * s;
    const x = c * (1 - Math.abs((hue / 60) % 2 - 1));
    const m = l - c / 2;
    let r=0, g=0, b=0;
    
    if (0 <= hue && hue < 60) { r = c; g = x; b = 0; }
    else if (60 <= hue && hue < 120) { r = x; g = c; b = 0; }
    else if (120 <= hue && hue < 180) { r = 0; g = c; b = x; }
    else if (180 <= hue && hue < 240) { r = 0; g = x; b = c; }
    else if (240 <= hue && hue < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return [Math.round((r + m) * 255), Math.round((g + m) * 255), Math.round((b + m) * 255)];
}

function render() {
    const canvas = document.getElementById('particleCanvas');
    const ctx = canvas.getContext('2d');
    const imageData = ctx.createImageData(WIDTH, HEIGHT);
    const data = imageData.data;
    const len = renderData.length;
    
    if (renderMode === 'type') {
        for (let i = 0; i < len; i++) {
            const type = renderData[i];
            const color = colors[type] || [255, 0, 255];
            const idx = i << 2;
            data[idx] = color[0];
            data[idx+1] = color[1];
            data[idx+2] = color[2];
            data[idx+3] = 255;
        }
    } else {
        // Temperature mode
        for (let i = 0; i < len; i++) {
            const offset = particleData + i * particleSize;
            const temp = wasmModule.HEAPF32[(offset + 4) >> 2];
            const color = getTempColor(temp);
            const idx = i << 2;
            data[idx] = color[0];
            data[idx+1] = color[1];
            data[idx+2] = color[2];
            data[idx+3] = 255;
        }
    }
    ctx.putImageData(imageData, 0, 0);
}

window.addEventListener('load', loadWasm);
