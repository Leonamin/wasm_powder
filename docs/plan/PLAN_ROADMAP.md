# 구현 로드맵 (PLAN_ROADMAP.md)

## 🗺️ 전체 구현 순서

이 문서는 PLAN_SUMMARY.md의 단계별 계획을 실제 구현 가능한 작은 태스크로 분해합니다.

---

## 📦 STEP 1: C++ 기반 구축

### 목표
핵심 데이터 구조 정의 및 MaterialDB 구축

### 세부 태스크

#### 1.1 Particle 구조체 확장
- [ ] `src/particle.h` 수정
  - `state` 필드 추가 (SOLID, POWDER, LIQUID, GAS)
  - `vx`, `vy` 필드 추가 (속도)
  - `latent_heat_storage` 필드 추가
  - `life` 필드 추가
  - `updated_this_frame` 플래그 추가

**예상 코드:**
```cpp
enum PhysicalState {
    SOLID = 0,
    POWDER = 1,
    LIQUID = 2,
    GAS = 3
};

struct Particle {
    // 1. 물질 그리드
    int type;
    
    // 2. 환경 그리드
    float temperature;
    
    // 3. 물리 상태
    int state;
    float vx, vy;
    float latent_heat_storage;
    
    // 4. 기타
    int life;
    bool updated_this_frame;
    
    Particle() : type(EMPTY), temperature(20.0f), state(SOLID),
                 vx(0), vy(0), latent_heat_storage(0),
                 life(-1), updated_this_frame(false) {}
};
```

#### 1.2 MaterialDB 구조 설계
- [ ] `src/material_db.h` 생성
- [ ] `Material` 구조체 정의
- [ ] 물질 속성 상수 정의

**예상 코드:**
```cpp
struct Material {
    const char* name;
    int default_state;        // SOLID, POWDER, LIQUID, GAS
    float density;            // kg/m³
    float specific_heat;      // J/(kg·K)
    float melting_point;      // °C
    float boiling_point;      // °C
    float latent_heat_fusion; // J/kg (융해열)
    float latent_heat_vaporization; // J/kg (기화열)
    float viscosity;          // Pa·s
    int color[3];             // RGB
};
```

#### 1.3 기본 물질 3개 정의
- [ ] EMPTY (공기)
- [ ] WALL (벽)
- [ ] SAND (모래)

**예상 데이터:**
```cpp
const Material g_MaterialDB[] = {
    // EMPTY (공기)
    {
        .name = "Air",
        .default_state = GAS,
        .density = 1.2f,
        .specific_heat = 1005.0f,
        .melting_point = -999.0f,
        .boiling_point = -999.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {0, 0, 0}
    },
    // WALL
    {
        .name = "Wall",
        .default_state = SOLID,
        .density = 2500.0f,
        .specific_heat = 840.0f,
        .melting_point = 1500.0f,
        .boiling_point = 2800.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {136, 136, 136}
    },
    // SAND
    {
        .name = "Sand",
        .default_state = POWDER,
        .density = 1600.0f,
        .specific_heat = 830.0f,
        .melting_point = 1700.0f,
        .boiling_point = 2230.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {240, 230, 140}
    }
};
```

#### 1.4 검증
- [ ] C++ `main()` 함수 작성 (테스트용)
- [ ] MaterialDB 조회 테스트
- [ ] Particle 생성 및 속성 확인

---

## 🔗 STEP 2: 기본 연동 및 배경 렌더링

### 목표
JS가 Wasm의 Particle 배열을 읽고 온도를 시각화

### 세부 태스크

#### 2.1 C++ 함수 노출
- [ ] `getParticleArrayPtr()` 함수 추가
  - Particle 배열 전체의 포인터 반환
- [ ] `getParticleSize()` 함수 추가
  - sizeof(Particle) 반환 (JS에서 구조체 크기 확인용)

**예상 코드:**
```cpp
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    Particle* getParticleArrayPtr() {
        return grid;
    }
    
    EMSCRIPTEN_KEEPALIVE
    int getParticleSize() {
        return sizeof(Particle);
    }
}
```

#### 2.2 JS에서 Particle 배열 매핑
- [ ] `main.js` 수정
- [ ] Particle 구조체를 JS에서 읽을 수 있도록 매핑
- [ ] 온도 데이터 추출 함수 작성

**예상 코드:**
```javascript
// Particle 구조체 크기 계산
const particleSize = wasmModule._getParticleSize();
const particlePtr = wasmModule._getParticleArrayPtr();

// Particle 배열 접근 함수
function getParticle(x, y) {
    const idx = y * WIDTH + x;
    const offset = particlePtr + idx * particleSize;
    
    // 구조체 필드 오프셋 (C++ 구조체와 일치해야 함)
    const type = Module.HEAP32[(offset + 0) >> 2];
    const temperature = Module.HEAPF32[(offset + 4) >> 2];
    // ... 다른 필드들
    
    return { type, temperature };
}
```

#### 2.3 렌더링 모드 UI 추가
- [ ] HTML에 렌더링 모드 선택 버튼 추가
  - "물질 타입 보기"
  - "온도 보기"
- [ ] CSS 스타일링

**예상 HTML:**
```html
<div class="render-mode">
    <button class="mode-btn active" data-mode="type">물질 타입</button>
    <button class="mode-btn" data-mode="temperature">온도</button>
</div>
```

#### 2.4 온도 시각화 렌더링
- [ ] 온도 → 색상 변환 함수 작성
  - 0°C = 파랑 (HSL: 240°)
  - 50°C = 초록 (HSL: 120°)
  - 100°C = 빨강 (HSL: 0°)
- [ ] `render()` 함수에 모드별 렌더링 추가

**예상 코드:**
```javascript
function temperatureToColor(temp) {
    // -20°C ~ 150°C를 0~1로 정규화
    const normalized = (temp + 20) / 170;
    const clamped = Math.max(0, Math.min(1, normalized));
    
    // HSL: 파랑(240) → 빨강(0)
    const hue = (1 - clamped) * 240;
    
    return hslToRgb(hue, 100, 50);
}
```

#### 2.5 검증
- [ ] 빌드 후 브라우저에서 확인
- [ ] 모든 셀이 20°C (초록색)로 표시되는지 확인
- [ ] FIRE 입자 추가 시 빨간색으로 표시되는지 확인

---

## 🔥 STEP 3: 열 전도 구현

### 목표
EMPTY 셀을 통해 열이 자연스럽게 퍼지도록 구현

### 세부 태스크

#### 3.1 update() 함수 리팩토링
- [ ] `simulation.cpp`의 `update()` 함수를 다중 패스로 분리
- [ ] 각 패스를 별도 함수로 추출

**예상 구조:**
```cpp
void update() {
    // PASS 0: 준비
    memcpy(nextGrid, grid, sizeof(grid));
    
    // PASS 1: 화학 반응 (나중에 구현)
    // updateChemistry();
    
    // PASS 2: 열 전도
    updateHeatConduction();
    
    // PASS 3: 상태 전이 (기존 코드 이동)
    updateStateChange();
    
    // PASS 4: 힘 계산 (나중에 구현)
    // updateForces();
    
    // PASS 5: 이동 (기존 코드 이동)
    updateMovement();
    
    // FINAL
    memcpy(grid, nextGrid, sizeof(grid));
    updateRenderBuffer();
}
```

#### 3.2 열 전도 로직 구현
- [ ] `updateHeatConduction()` 함수 작성
- [ ] 주변 4칸의 온도 평균 계산
- [ ] MaterialDB의 비열(specific_heat) 고려

**예상 코드:**
```cpp
void updateHeatConduction() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = getIndex(x, y);
            Particle& p = grid[idx];
            
            // 주변 4칸의 온도 수집
            float neighborTemps[4];
            int count = 0;
            
            if (inBounds(x, y-1)) neighborTemps[count++] = grid[getIndex(x, y-1)].temperature;
            if (inBounds(x, y+1)) neighborTemps[count++] = grid[getIndex(x, y+1)].temperature;
            if (inBounds(x-1, y)) neighborTemps[count++] = grid[getIndex(x-1, y)].temperature;
            if (inBounds(x+1, y)) neighborTemps[count++] = grid[getIndex(x+1, y)].temperature;
            
            // 평균 온도 계산
            float avgTemp = 0;
            for (int i = 0; i < count; i++) {
                avgTemp += neighborTemps[i];
            }
            avgTemp /= count;
            
            // 비열 고려한 열 전도 (간단한 버전)
            const Material& mat = g_MaterialDB[p.type];
            float conductionRate = 0.1f / mat.specific_heat;
            
            nextGrid[idx].temperature = p.temperature + (avgTemp - p.temperature) * conductionRate;
        }
    }
}
```

#### 3.3 Active Chunks 기초 구현
- [ ] 청크 시스템 설계 (16x16 또는 32x32)
- [ ] 변화가 있는 청크만 마킹
- [ ] update()에서 활성 청크만 순회

**예상 코드:**
```cpp
const int CHUNK_SIZE = 16;
const int CHUNK_WIDTH = (WIDTH + CHUNK_SIZE - 1) / CHUNK_SIZE;
const int CHUNK_HEIGHT = (HEIGHT + CHUNK_SIZE - 1) / CHUNK_SIZE;
bool activeChunks[CHUNK_WIDTH * CHUNK_HEIGHT];

void markChunkActive(int x, int y) {
    int cx = x / CHUNK_SIZE;
    int cy = y / CHUNK_SIZE;
    activeChunks[cy * CHUNK_WIDTH + cx] = true;
}
```

#### 3.4 검증
- [ ] FIRE 입자 하나 추가
- [ ] 온도 보기 모드에서 열이 퍼지는지 확인
- [ ] FIRE 제거 후에도 열이 남아있는지 확인
- [ ] 시간이 지나면 서서히 식는지 확인

---

## 💧 STEP 4: 밀도 & 이동 구현

### 목표
밀도 기반 교환과 중력 구현

### 세부 태스크

#### 4.1 WATER 물질 추가
- [ ] MaterialDB에 WATER 추가
  - density: 1000.0
  - melting_point: 0.0
  - boiling_point: 100.0

#### 4.2 힘 계산 패스 구현
- [ ] `updateForces()` 함수 작성
- [ ] 중력 적용 (밀도 고려)
- [ ] 부력 계산

**예상 코드:**
```cpp
void updateForces() {
    const float GRAVITY = 0.5f;
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = getIndex(x, y);
            Particle& p = grid[idx];
            
            if (p.type == EMPTY || p.type == WALL) continue;
            if (p.state == SOLID) continue;
            
            const Material& mat = g_MaterialDB[p.type];
            
            // 중력 (밀도에 비례)
            nextGrid[idx].vy += GRAVITY * (mat.density / 1000.0f);
        }
    }
}
```

#### 4.3 이동 패스 개선
- [ ] 기존 `updateParticle()` 로직을 `updateMovement()`로 이동
- [ ] 밀도 비교 후 교환 로직 추가
- [ ] 속도(vx, vy) 기반 이동

**예상 코드:**
```cpp
void updateMovement() {
    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = getIndex(x, y);
            Particle& p = grid[idx];
            
            if (p.type == EMPTY || p.type == WALL) continue;
            if (p.updated_this_frame) continue;
            
            const Material& mat = g_MaterialDB[p.type];
            
            // 속도 기반 목표 위치 계산
            int targetX = x + (int)p.vx;
            int targetY = y + (int)p.vy;
            
            if (!inBounds(targetX, targetY)) continue;
            
            Particle& target = grid[getIndex(targetX, targetY)];
            
            // 밀도 비교
            if (target.type == EMPTY || mat.density > g_MaterialDB[target.type].density) {
                // 교환 (온도도 함께 이동)
                swapParticlesWithTemp(x, y, targetX, targetY);
            }
            
            // 속도 감쇠
            nextGrid[idx].vx *= 0.9f;
            nextGrid[idx].vy *= 0.9f;
        }
    }
}
```

#### 4.4 온도 이동 로직 수정
- [ ] `swapParticles()` 함수 수정
- [ ] 입자가 이동할 때 온도도 함께 이동

**예상 코드:**
```cpp
void swapParticlesWithTemp(int x1, int y1, int x2, int y2) {
    int idx1 = getIndex(x1, y1);
    int idx2 = getIndex(x2, y2);
    
    // 전체 Particle 교환 (온도 포함)
    Particle temp = nextGrid[idx1];
    nextGrid[idx1] = nextGrid[idx2];
    nextGrid[idx2] = temp;
    
    nextGrid[idx1].updated_this_frame = true;
    nextGrid[idx2].updated_this_frame = true;
}
```

#### 4.5 검증
- [ ] SAND와 WATER 동시에 추가
- [ ] SAND가 WATER에 가라앉는지 확인
- [ ] 뜨거운 SAND가 차가운 WATER를 데우는지 확인 (온도 보기 모드)

---

## 🎨 STEP 5: 프론트엔드 개선

### 목표
마우스 pressed 버그 수정 및 UI 개선

### 세부 태스크

#### 5.1 마우스 Pressed 버그 수정
- [ ] `main.js` 수정
- [ ] `requestAnimationFrame` 기반 연속 입자 생성

**예상 코드:**
```javascript
let isDrawing = false;
let lastMouseX = 0;
let lastMouseY = 0;

function continuousDrawLoop() {
    if (isDrawing) {
        addParticleAt(lastMouseX, lastMouseY);
    }
    requestAnimationFrame(continuousDrawLoop);
}

canvas.addEventListener('mousedown', (e) => {
    isDrawing = true;
    updateMousePosition(e);
    addParticleAtMouse(e);
});

canvas.addEventListener('mousemove', (e) => {
    updateMousePosition(e);
});

canvas.addEventListener('mouseup', () => {
    isDrawing = false;
});

function updateMousePosition(e) {
    const rect = canvas.getBoundingClientRect();
    lastMouseX = Math.floor(e.clientX - rect.left);
    lastMouseY = Math.floor(e.clientY - rect.top);
}

// 게임 루프와 별도로 실행
continuousDrawLoop();
```

#### 5.2 FPS 표시 추가
- [ ] HTML에 FPS 표시 영역 추가
- [ ] JS에서 FPS 계산 및 업데이트

#### 5.3 브러시 크기 조절
- [ ] HTML에 슬라이더 추가
- [ ] JS에서 brushSize 변수 조절

#### 5.4 검증
- [ ] 마우스를 누르고 있으면 계속 입자가 생성되는지 확인
- [ ] 움직이지 않아도 같은 위치에 계속 추가되는지 확인

---

## 🧪 STEP 6: 고급 기능 (확장)

### 목표
잠열, 점도, 화학 반응 구현

### 세부 태스크

#### 6.1 잠열 시스템
- [ ] `updateStateChange()` 수정
- [ ] `latent_heat_storage` 축적 로직
- [ ] 잠열 초과 시 상태 전이

#### 6.2 점도 시스템
- [ ] `updateMovement()`에 점도 로직 추가
- [ ] 액체의 퍼짐 속도를 점도로 조절

#### 6.3 화학 반응
- [ ] `ChemistryDB` 구조 설계
- [ ] `updateChemistry()` 함수 구현

---

## ✅ 완료 체크리스트

### STEP 1
- [ ] Particle 구조체 확장
- [ ] MaterialDB 구조 정의
- [ ] 기본 물질 3개 정의
- [ ] 검증 완료

### STEP 2
- [ ] C++ 함수 노출
- [ ] JS Particle 매핑
- [ ] 렌더링 모드 UI
- [ ] 온도 시각화
- [ ] 검증 완료

### STEP 3
- [ ] update() 리팩토링
- [ ] 열 전도 구현
- [ ] Active Chunks
- [ ] 검증 완료

### STEP 4
- [ ] WATER 추가
- [ ] 힘 계산 패스
- [ ] 이동 패스 개선
- [ ] 온도 이동
- [ ] 검증 완료

### STEP 5
- [ ] 마우스 버그 수정
- [ ] FPS 표시
- [ ] 브러시 크기 조절
- [ ] 검증 완료

### STEP 6
- [ ] 잠열 시스템
- [ ] 점도 시스템
- [ ] 화학 반응

---

## 📝 참고사항

- 각 STEP은 독립적으로 테스트 가능해야 함
- 빌드 후 반드시 브라우저에서 검증
- 성능 문제 발생 시 Active Chunks 우선 최적화
- 버그 발견 시 PLAN_BUG.md에 기록
