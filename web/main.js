// 전역 변수
let wasmModule = null;
let renderData = null;
let selectedType = 2; // 기본값: SAND
let isDrawing = false;
let WIDTH = 400;
let HEIGHT = 300;

// 입자 타입별 색상 (RGB)
const colors = {
    0: [0, 0, 0],           // EMPTY - 검정
    1: [136, 136, 136],     // WALL - 회색
    2: [240, 230, 140],     // SAND - 카키색
    3: [30, 144, 255],      // WATER - 파랑
    4: [175, 238, 238],     // ICE - 하늘색
    5: [245, 245, 245],     // STEAM - 흰색
    6: [255, 69, 0],        // FIRE - 주황/빨강
    7: [65, 105, 225]       // FROST - 로얄블루
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
            
            // UI 초기화
            initUI();
            
            // 로딩 화면 숨기고 앱 표시
            document.getElementById('loading').style.display = 'none';
            document.getElementById('app').style.display = 'block';
            
            // 게임 루프 시작
            gameLoop();
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
    
    // 입자 선택 버튼 이벤트
    document.querySelectorAll('.particle-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.particle-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            selectedType = parseInt(btn.dataset.type);
        });
    });
    
    // 마우스 이벤트
    canvas.addEventListener('mousedown', (e) => {
        isDrawing = true;
        addParticleAtMouse(e);
    });
    
    canvas.addEventListener('mouseup', () => {
        isDrawing = false;
    });
    
    canvas.addEventListener('mouseleave', () => {
        isDrawing = false;
    });
    
    canvas.addEventListener('mousemove', (e) => {
        if (isDrawing) {
            addParticleAtMouse(e);
        }
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

// 마우스 위치에 입자 추가
function addParticleAtMouse(e) {
    const canvas = document.getElementById('particleCanvas');
    const rect = canvas.getBoundingClientRect();
    const x = Math.floor(e.clientX - rect.left);
    const y = Math.floor(e.clientY - rect.top);
    addParticleAt(x, y);
}

// 좌표에 입자 추가 (브러시 효과)
function addParticleAt(x, y) {
    const brushSize = 3; // 브러시 크기
    
    for (let dy = -brushSize; dy <= brushSize; dy++) {
        for (let dx = -brushSize; dx <= brushSize; dx++) {
            // 원형 브러시
            if (dx * dx + dy * dy <= brushSize * brushSize) {
                const px = x + dx;
                const py = y + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    wasmModule._addParticle(px, py, selectedType);
                }
            }
        }
    }
}

// 그리드 전체 지우기
function clearGrid() {
    wasmModule._init();
}

// 메인 게임 루프
function gameLoop() {
    // Wasm 시뮬레이션 업데이트
    wasmModule._update();
    
    // 캔버스 렌더링
    render();
    
    // 다음 프레임
    requestAnimationFrame(gameLoop);
}

// 렌더링
function render() {
    const canvas = document.getElementById('particleCanvas');
    const ctx = canvas.getContext('2d');
    
    // ImageData를 사용한 빠른 렌더링
    const imageData = ctx.createImageData(WIDTH, HEIGHT);
    const data = imageData.data;
    
    for (let i = 0; i < renderData.length; i++) {
        const type = renderData[i];
        const color = colors[type] || [255, 0, 255]; // 기본값: 마젠타
        
        const idx = i * 4;
        data[idx] = color[0];     // R
        data[idx + 1] = color[1]; // G
        data[idx + 2] = color[2]; // B
        data[idx + 3] = 255;      // A
    }
    
    ctx.putImageData(imageData, 0, 0);
}

// 페이지 로드 시 Wasm 로드
window.addEventListener('load', loadWasm);
