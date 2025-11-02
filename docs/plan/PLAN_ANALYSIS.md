# 프로젝트 분석 (PLAN_ANALYSIS.md)

## 📊 현재 상태 분석 (1차 MVP)

### 구현된 기능

#### 1. **C++ 시뮬레이션 엔진** (`src/simulation.cpp`)
- ✅ 기본 그리드 시스템 (400x300)
- ✅ 이중 버퍼링 (`grid`, `nextGrid`)
- ✅ 7가지 입자 타입 구현
  - EMPTY, WALL, SAND, WATER, ICE, STEAM, FIRE, FROST
- ✅ 온도 시스템 (기본)
- ✅ 상태 전이 (물 ↔ 얼음 ↔ 증기)
- ✅ 입자 이동 로직 (중력, 확산)

#### 2. **입자 구조체** (`src/particle.h`)
```cpp
struct Particle {
    int type;          // 입자 타입
    float temperature; // 온도
};
```

#### 3. **프론트엔드** (`web/`)
- ✅ WebAssembly 모듈 로딩
- ✅ Canvas 렌더링 (ImageData 사용)
- ✅ 마우스/터치 입력
- ✅ 브러시 시스템 (원형, 크기 3)
- ✅ 입자 선택 UI

---

## 🔍 현재 구조의 문제점

### 1. **아키텍처 문제**

#### A. 모놀리식 업데이트 로직
- 모든 물리 법칙이 `updateParticle()` 함수 하나에 뭉쳐있음
- 화학 반응, 열 전도, 상태 전이, 이동이 분리되지 않음
- 새로운 물질 추가 시 코드 수정이 어려움

#### B. 하드코딩된 물질 속성
```cpp
// 현재: 속성이 코드에 직접 박혀있음
case FIRE:
    grid[idx].temperature = 150.0f;  // 하드코딩
    break;
```

- 물질의 밀도, 비열, 녹는점 등이 코드에 직접 작성됨
- 물질 추가/수정 시 여러 곳을 수정해야 함
- 데이터와 로직이 분리되지 않음

#### C. 제한적인 물리 시뮬레이션
- **밀도 시스템 없음**: 모래가 물에 가라앉지 않음
- **열 전도 없음**: 온도가 주변으로 자연스럽게 퍼지지 않음
- **잠열 없음**: 얼음이 0°C에서 즉시 녹음
- **점도 없음**: 모든 액체가 같은 속도로 퍼짐
- **화학 반응 없음**: 물질 간 상호작용 불가

#### D. 배경(EMPTY) 개념 부재
- EMPTY 셀은 단순히 "빈 공간"일 뿐
- EMPTY 셀이 온도를 가지지 않음 (온도가 사라짐)
- 열이 빈 공간을 통해 전달되지 않음

```cpp
// 현재 문제
moveParticle(x, y, x, y+1) {
    nextGrid[toIdx] = grid[fromIdx];
    nextGrid[fromIdx].type = EMPTY;
    nextGrid[fromIdx].temperature = 20.0f;  // 온도가 사라짐!
}
```

### 2. **프론트엔드 문제**

#### A. 마우스 Pressed 버그 (🐛 PLAN_BUG.md #1)
- 마우스를 누르고 있어도 움직이지 않으면 입자가 생성되지 않음

#### B. 제한적인 시각화
- 입자 타입만 표시 (온도 시각화 없음)
- 디버깅 정보 부족
- 렌더링 모드 선택 불가

---

## 🎯 새로운 아키텍처 (PLAN_SUMMARY.md 기반)

### 핵심 개념

#### 1. **병렬 그리드 (Parallel Grids)**
```
grid[x][y] = {
    type: SAND,           // 물질 그리드
    temperature: 25.0,    // 온도 그리드
    pressure: 1.0,        // 기압 그리드 (Phase 3)
    vx, vy,              // 속도 그리드
    ...
}
```

- **모든 셀**이 온도를 가짐 (EMPTY 포함)
- EMPTY는 "공기"로 취급
- 열이 배경을 통해 전달됨

#### 2. **MaterialDB (물질 속성 데이터베이스)**
```cpp
struct Material {
    const char* name;
    float density;           // 밀도
    float specific_heat;     // 비열
    float melting_point;     // 녹는점
    float boiling_point;     // 끓는점
    float latent_heat_fusion; // 융해 잠열
    float viscosity;         // 점도
    int default_color[3];    // 기본 색상
};

Material g_MaterialDB[MAX_MATERIALS];
```

#### 3. **다중 패스 업데이트 시스템**
```cpp
void update() {
    // PASS 0: 준비
    memcpy(nextGrid, grid, sizeof(grid));
    
    // PASS 1: 화학 반응 & 생명주기
    // - 수명 감소
    // - 화학 반응 처리
    
    // PASS 2: 환경 물리 (열 전도)
    // - 모든 셀의 온도 확산 (EMPTY 포함)
    // - 비열 고려
    
    // PASS 3: 상태 전이
    // - 잠열 축적
    // - 상태 변경 (고체 ↔ 액체 ↔ 기체)
    
    // PASS 4: 힘 계산
    // - 중력/부력 (밀도 기반)
    
    // PASS 5: 이동 및 교환
    // - 밀도 비교 후 교환
    // - 점도 기반 퍼짐
    // - 온도를 함께 이동
    
    // FINAL: 그리드 교체
    memcpy(grid, nextGrid, sizeof(grid));
}
```

---

## 📋 구현 우선순위

### Phase 1: 기반 구축 (현재 → STEP 1-2)
1. ✅ Particle 구조체 확장
2. ✅ MaterialDB 구조 설계
3. ✅ 기본 3개 물질 정의 (EMPTY, WALL, SAND)
4. ✅ JS-Wasm 연동 개선

### Phase 2: 배경 시스템 (STEP 3)
1. 열 전도 구현 (PASS 2)
2. EMPTY 셀의 온도 유지
3. 온도 시각화 UI
4. Active Chunks 최적화

### Phase 3: 물리 시스템 (STEP 4)
1. 밀도 기반 이동
2. 속도 시스템 (vx, vy)
3. 물질 추가 (WATER, 밀도 차이 확인)

### Phase 4: 고급 기능 (STEP 5-6)
1. 잠열 시스템
2. 점도 시스템
3. 화학 반응
4. 기압 시스템 (PLAN_PRESSURE.md)

---

## 🔧 리팩토링 계획

### 1. **Particle 구조체 확장**
```cpp
struct Particle {
    // 1. 물질 그리드
    int type;
    
    // 2. 환경 그리드 (배경 속성)
    float temperature;
    // float pressure; // Phase 3
    
    // 3. 물리 상태
    int state; // SOLID, POWDER, LIQUID, GAS
    float vx, vy;
    float latent_heat_storage;
    
    // 4. 기타
    int life;
    bool updated_this_frame;
};
```

### 2. **MaterialDB 구축**
- `material_db.h` 파일 생성
- 물질 속성 테이블 정의
- 조회 함수 구현

### 3. **update() 루프 분리**
- `updateChemistry()` - PASS 1
- `updateHeatConduction()` - PASS 2
- `updateStateChange()` - PASS 3
- `updateForces()` - PASS 4
- `updateMovement()` - PASS 5

---

## 📈 성능 고려사항

### 현재 성능
- 400x300 = 120,000 셀
- 단일 패스 업데이트
- ~60 FPS 가능

### 다중 패스 후 예상
- 5개 패스 → 5배 연산량
- Active Chunks 최적화 필수
- 목표: 30-60 FPS 유지

### 최적화 전략
1. **Active Chunks**: 변화가 있는 영역만 업데이트
2. **SIMD**: Emscripten SIMD 플래그 활용
3. **캐시 친화적 순회**: 메모리 접근 최적화
4. **조기 종료**: EMPTY 영역 스킵

---

## 🎨 프론트엔드 개선 계획

### 1. 마우스 Pressed 버그 수정
```javascript
let drawInterval = null;
let lastMousePos = {x: 0, y: 0};

canvas.addEventListener('mousedown', (e) => {
    isDrawing = true;
    updateMousePos(e);
    addParticleAtMouse(e);
    
    // 지속적으로 입자 생성
    drawInterval = setInterval(() => {
        if (isDrawing) {
            addParticleAt(lastMousePos.x, lastMousePos.y);
        }
    }, 16); // ~60fps
});

canvas.addEventListener('mouseup', () => {
    isDrawing = false;
    clearInterval(drawInterval);
});
```

### 2. 렌더링 모드 추가
- **물질 타입 보기** (현재)
- **온도 보기** (HSL 색상 스케일)
- **속도 보기** (화살표 또는 색상)
- **디버그 모드** (그리드 라인, 정보 표시)

### 3. UI 개선
- FPS 표시
- 시뮬레이션 속도 조절
- 브러시 크기 조절
- 물질 정보 툴팁

---

## 📝 다음 단계

1. ✅ **버그 문서 작성** (PLAN_BUG.md)
2. **MaterialDB 설계 및 구현**
3. **Particle 구조체 확장**
4. **다중 패스 시스템 구현**
5. **프론트엔드 버그 수정**
6. **온도 시각화 추가**

---

## 🔗 관련 문서
- [PLAN_SUMMARY.md](./PLAN_SUMMARY.md) - 전체 계획
- [PLAN_PRESSURE.md](./PLAN_PRESSURE.md) - 기압 시스템
- [PLAN_BUG.md](./PLAN_BUG.md) - 버그 목록
- [GUIDE.md](../GUIDE.md) - 1차 MVP 가이드
