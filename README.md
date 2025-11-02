# 1\. 프로젝트 개요

  * **프로젝트명:** Wasm 파우더 토이 (Wasm Particle Toy)
  * **프로젝트 목표:** C++/WebAssembly를 사용하여 대규모 2D 입자 시뮬레이터를 구현, 순수 JS 대비 Wasm의 성능적 우위를 입증한다.
  * **핵심 컨셉:**
    1.  C++(Wasm)이 **모든 입자의 물리 연산과 상태 변화(온도 포함)**를 담당한다.
    2.  JS는 Wasm의 연산 결과를 받아 **화면에 렌더링**하고, **사용자 입력**을 Wasm에 전달한다.
  * **팀 역할 (제안):**
      * **서버 개발자:** C++ 로직(시뮬레이션 엔진, 입자 구조) 전체 담당.
      * **앱 개발자:** JS 로직(Wasm 로딩, Canvas 렌더링, UI/입력) 전체 담당.
      * **비개발자:** 기획 보조, 순수 JS 버전 제작(성능 비교용), 최종 발표 자료 제작.
## 환경 설정

### 1. Windows (WSL 사용 권장)

**WSL 설치:**
```powershell
# PowerShell 관리자 권한으로 실행
wsl --install
```

**Ubuntu 초기 설정:**
```bash
# 패키지 업데이트
sudo apt update && sudo apt upgrade -y

# 필수 도구 설치
sudo apt install -y build-essential cmake python3 git

# Emscripten 설치
sudo apt install -y emscripten

# 줄바꿈 문제 해결 도구
sudo apt install -y dos2unix
```

**프로젝트 접근 및 빌드:**
```bash
# Windows 파일 시스템 접근 (만약 VS Code에서 터미널을 사용한다면 바로 현재 폴더가 열려서 건너 뛰어도 됨)
cd /mnt/c/dev/wasm_powder

# 줄바꿈 문제 해결 (최초 1회)
dos2unix build.sh

# 빌드 실행
chmod +x build.sh
./build.sh
```

**웹 서버 실행:**
```bash
cd web
python3 -m http.server 8000
# 브라우저에서 http://localhost:8000 접속
```

> 💡 **팁**: VS Code에서 "WSL" 확장을 설치하면 WSL 환경에서 직접 개발 가능

---

### 2. macOS

**Homebrew 설치 (없는 경우):**
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**필수 도구 설치:**
```bash
# Xcode Command Line Tools (없는 경우)
xcode-select --install

# Emscripten 설치
brew install emscripten

# Python3 (보통 기본 설치되어 있음)
brew install python3
```

**빌드 및 실행:**
```bash
cd /path/to/wasm_powder

# 빌드
chmod +x build.sh
./build.sh

# 빌드된 웹 파일 위치로 이동
cd web

# 웹 서버 실행
python3 -m http.server 8000
# 브라우저에서 http://localhost:8000 접속
```

---

### 3. Linux (Ubuntu/Debian)

**필수 도구 설치:**
```bash
# 패키지 업데이트
sudo apt update && sudo apt upgrade -y

# 빌드 도구
sudo apt install -y build-essential cmake git

# Python3
sudo apt install -y python3

# Emscripten
sudo apt install -y emscripten
```

**빌드 및 실행:**
```bash
cd /path/to/wasm_powder

# 빌드
chmod +x build.sh
./build.sh

# 빌드된 웹 파일 위치로 이동
cd web

# 웹 서버 실행
python3 -m http.server 8000
# 브라우저에서 http://localhost:8000 접속
```

---

### 공통 문제 해결

**1. `bad interpreter: /bin/bash^M` 에러 (Windows/WSL)**
```bash
dos2unix build.sh
```

**2. Emscripten 버전 확인**
```bash
emcc --version
```

**3. 빌드 실패 시 디버깅**
```bash
# 상세 로그 출력
emcc src/simulation.cpp -o web/simulation.js -v
```

-----

# 구현 계획
## 2\. Phase 1: 즉시 구현할 핵심 기능 (MVP)

복잡한 시뮬레이션(열 전도, 기압)은 **제외**하고, **'온도' 변수**와 **'상태 전이'** 규칙에 집중합니다.

### 가. 핵심 데이터 구조 (C++ / 서버 개발자)

모든 확장의 기반이 될 `Particle` 구조체를 정의합니다.

```cpp
// Particle.h (예시)
struct Particle {
    // 1. 입자 타입 (JS 렌더링의 핵심)
    // 0=EMPTY, 1=WALL, 2=SAND, 3=WATER, 4=ICE, 5=STEAM, 6=FIRE, 7=FROST
    int type = 0;

    // 2. 입자 온도 (상태 전이의 핵심)
    // 섭씨(Celsius) 기준. 기본값은 상온 20도.
    float temperature = 20.0f; 

    // 3. (미래 확장용)
    // float density = 1.0f; // 밀도 (Phase 2)
    // int life = -1;       // 수명 (Phase 2)
};
```

  * **메모리 구조 (C++):**
      * `const int WIDTH = 400, HEIGHT = 300;`
      * `Particle grid[WIDTH * HEIGHT];` (현재 상태)
      * `Particle nextGrid[WIDTH * HEIGHT];` (다음 프레임 계산용)
      * `int render_buffer[WIDTH * HEIGHT];` (**JS 전달 전용 버퍼**)

### 나. Phase 1 구현 입자 (총 8개)

1.  `EMPTY` (0): 빈 공간
2.  `WALL` (1): **[고정 고체]** 움직이지 않음.
3.  `SAND` (2): **[가루 고체]** 중력(아래)으로 떨어짐. 물에 가라앉음.
4.  `WATER` (3): **[액체]** 중력으로 떨어지며, 막히면 옆으로 퍼짐.
5.  `ICE` (4): **[고정 고체]** `WATER`가 0°C 미만이 되면 변함. 0°C 초과 시 `WATER`로 변함.
6.  `STEAM` (5): **[기체]** `WATER`가 100°C 초과 시 변함. 위(반중력)로 올라가며 옆으로 퍼짐. 100°C 미만 시 `WATER`로 변함.
7.  `FIRE` (6): **[특수/온도원]** 주변 8칸의 입자를 `nextGrid`에서 150°C로 만듦. (열 전도 X, 강제 설정 O)
8.  `FROST` (7): **[특수/온도원]** 주변 8칸의 입자를 `nextGrid`에서 -20°C로 만듦.

### 다. 핵심 로직 (C++ / 서버 개발자)

`update()` 함수가 엔진의 심장입니다.

```cpp
// Simulation.cpp (예시)

// Wasm이 JS로 내보낼(export) 함수들
extern "C" {
  // 1. 시뮬레이션 1프레임 실행
  void update() {
    // 1. nextGrid를 grid 상태로 복사 (기본 상태 유지)
    memcpy(nextGrid, grid, sizeof(grid));

    // 2. 모든 입자 순회 (핵심 로직)
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        updateParticle(x, y); // 각 입자별 규칙 적용
      }
    }
    
    // 3. 계산 완료된 nextGrid를 grid로 복사
    memcpy(grid, nextGrid, sizeof(grid));

    // 4. JS가 렌더링할 수 있게 render_buffer에 'type'만 복사
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
      render_buffer[i] = grid[i].type;
    }
  }

  // 2. JS가 렌더 버퍼의 주소를 가져갈 함수
  int* getRenderBufferPtr() {
    return render_buffer;
  }

  // 3. JS가 마우스로 입자를 추가할 함수
  void addParticle(int x, int y, int type) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    
    grid[y * WIDTH + x].type = type;
    // 타입에 따라 초기 온도 설정
    if (type == FIRE) grid[y * WIDTH + x].temperature = 150.0f;
    else if (type == FROST) grid[y * WIDTH + x].temperature = -20.0f;
    else if (type == ICE) grid[y * WIDTH + x].temperature = -10.0f;
    else grid[y * WIDTH + x].temperature = 20.0f;
  }
}

// 개별 입자 업데이트 로직
void updateParticle(int x, int y) {
    Particle p = grid[y * WIDTH + x]; // 현재 입자
    
    // 1. 온도에 따른 상태 전이 (가장 먼저 체크)
    switch (p.type) {
        case WATER:
            if (p.temperature >= 100.0f) { p.type = STEAM; }
            else if (p.temperature <= 0.0f) { p.type = ICE; }
            break;
        case ICE:
            if (p.temperature > 0.0f) { p.type = WATER; }
            break;
        case STEAM:
            if (p.temperature < 100.0f) { p.type = WATER; }
            break;
    }

    // 2. 타입별 움직임 (상태 전이 후의 타입 기준)
    switch (p.type) {
        case WALL:
        case ICE:
            // 고정 고체: 아무것도 안 함 (nextGrid에 이미 복사됨)
            break;
        case SAND:
            // [TODO] 아래(y+1)가 비었는지 확인 후 이동
            break;
        case WATER:
            // [TODO] 아래(y+1) -> 대각선 아래 -> 좌우(x-1, x+1) 순서로 이동
            break;
        case STEAM:
            // [TODO] 위(y-1) -> 대각선 위 -> 좌우(x-1, x+1) 순서로 이동
            break;
        case FIRE:
            // [TODO] 주변 8칸의 'nextGrid' 입자 온도를 150.0f로 설정
            break;
        case FROST:
            // [TODO] 주변 8칸의 'nextGrid' 입자 온도를 -20.0f로 설정
            break;
    }
    
    // [TODO] 위 로직의 결과를 바탕으로 'nextGrid[y*WIDTH+x]'에 최종 상태 쓰기
}
```

### 라. 렌더링 및 제어 (JS / 앱 개발자)

```javascript
// main.js (예시)
const WIDTH = 400;
const HEIGHT = 300;
const canvas = document.getElementById('particleCanvas');
const ctx = canvas.getContext('2d');
canvas.width = WIDTH;
canvas.height = HEIGHT;

// Wasm 모듈 로드 (wasmExports 객체를 얻었다고 가정)
const { update, getRenderBufferPtr, addParticle, memory } = wasmExports;

// Wasm 메모리를 직접 JS에서 읽기 위한 배열 (Int32Array 사용)
const gridPtr = getRenderBufferPtr();
const renderData = new Int32Array(memory.buffer, gridPtr, WIDTH * HEIGHT);

// 입자 타입별 색상
const colors = [
  '#000', // 0: EMPTY
  '#888', // 1: WALL
  '#f0e68c', // 2: SAND
  '#1e90ff', // 3: WATER
  '#afeeee', // 4: ICE
  '#f5f5f5', // 5: STEAM
  '#ff4500', // 6: FIRE
  '#4169e1', // 7: FROST
];

// 메인 렌더링 루프
function gameLoop() {
  // 1. Wasm에 시뮬레이션 1프레임 연산 요청
  update();

  // 2. 캔버스 렌더링 (ImageData 사용이 가장 빠름)
  const imageData = ctx.createImageData(WIDTH, HEIGHT);
  const data = imageData.data; // (R,G,B,A, R,G,B,A...)

  for (let i = 0; i < renderData.length; i++) {
    const type = renderData[i];
    // [TODO] 'colors[type]'의 헥사코드를 R,G,B로 변환하여 data 배열에 채우기
    // (예: data[i*4] = R, data[i*4+1] = G, ...)
  }
  ctx.putImageData(imageData, 0, 0);

  // 3. 다음 프레임 요청
  requestAnimationFrame(gameLoop);
}

// [TODO] 마우스 입력 이벤트 (mousedown, mousemove)
canvas.addEventListener('mousemove', (e) => {
  if (e.buttons === 1) { // 마우스 좌클릭 드래그
    // [TODO] 캔버스 좌표 (e.offsetX, e.offsetY)를 그리드 좌표 (x, y)로 변환
    const x = e.offsetX;
    const y = e.offsetY;
    const selectedType = parseInt(document.getElementById('particleSelector').value, 10);
    // 4. Wasm 함수 호출로 입자 추가
    addParticle(x, y, selectedType);
  }
});

// 루프 시작
gameLoop();
```

-----

## 3\. Phase 2: 추후 확장 계획

Phase 1이 성공적으로 완료된 후, 아래와 같이 확장합니다.

  * **가. 고급 물리 속성 추가:**
      * **목표:** 밀도(Density) 구현 (예: 기름, 흑요석).
      * **방법:** `Particle` 구조체에 `float density` 추가. `update()` 로직에서 입자 이동 시, 자신과 대상의 밀도를 비교. (예: `WATER`가 `SAND`를 만났을 때, `SAND.density > WATER.density` 이므로 서로 위치를 바꿈 → 모래가 가라앉음)
  * **나. 열 전도 시뮬레이션:**
      * **목표:** `FIRE` 입자 없이도 열이 자연스럽게 퍼지고 식도록 구현.
      * **방법:** `update()` 로직에 '열 계산 패스'를 '움직임 계산 패스'와 분리. 모든 입자가 주변 4칸 입자의 `temperature`를 읽어와 평균을 내어 `nextGrid`의 온도를 설정. (매우 CPU 집약적이므로 Wasm에 최적)
  * **다. 복잡한 상호작용:**
      * **목표:** 리튬 + 물 = 폭발, 불 + 기름 = 불 번짐.
      * **방법:** `update()` 로직에서 주변을 탐색하는 규칙 추가. (예: `updateWater`가 주변에 `LITHIUM`이 있는지 확인). '폭발'은 `addParticle`과 유사한 `applyExplosionForce(x, y)` 함수를 만들어 주변 입자를 강제로 밀어내는 방식으로 구현.
  * **라. 기압 (Pressure):**
      * **목표:** 기체의 움직임을 더 현실적으로 구현.
      * **방법:** `Particle` 구조체에 `float pressure` 추가. `update()`에 '기압 계산 패스' 추가. (난이도 최상)
  * **마. 입자 수명 (Life):**
      * **목표:** 연기, 스파크 등 일시적인 입자 구현.
      * **방법:** `Particle` 구조체에 `int life` 추가. `update()` 시 `life--`를 하고, `life == 0`이 되면 `EMPTY`로 변경.