# 브러시 시스템 구현 (BRUSH_SYSTEM.md)

**구현 날짜:** 2025-11-03

---

## 📋 개요

파우더 토이의 철학에 맞게 **물질(Material)**과 **상태 변화 도구(Tool)**를 명확히 분리했습니다.

### 핵심 개념

**물질 (Material):**
- SAND, WATER, ICE, STEAM, WALL, FIRE
- 영구적으로 존재
- 물리적 속성 보유

**도구 (Tool):**
- 가열 (Heat), 냉각 (Cool)
- 브러시로 적용
- 물질을 생성하지 않고 온도만 변경

---

## 🔥 FIRE 개선

### 문제점
- 불이 무한히 확산되어 화면 전체를 덮음

### 해결 방법
```cpp
// 부모 불보다 life 감소
if (rand() % 3 == 0 && p.life > 10) { // life가 10 이상일 때만 확산
    int newLife = p.life - 5 - rand() % 6; // 부모보다 5-10 감소
    if (newLife > 0) {
        nextGrid[nIdx].type = FIRE;
        nextGrid[nIdx].life = newLife;
    }
}
```

### 효과
- ✅ 불이 2-3세대 확산 후 자연스럽게 소멸
- ✅ 초기 불: 30-60 프레임
- ✅ 1세대: 20-50 프레임
- ✅ 2세대: 10-40 프레임
- ✅ 3세대: 0-30 프레임 → 대부분 확산 불가

---

## ❄️ FROST 제거 → 냉각 브러시로 변경

### 이전 (잘못된 구현)
```cpp
// FROST를 물질로 생성
grid[idx].type = FROST;
grid[idx].temperature = -20.0f;
```

**문제점:**
- FROST가 물질처럼 쌓임
- 온도를 덮어써서 열 전도 차단
- 파우더 토이의 철학과 맞지 않음

### 현재 (올바른 구현)
```javascript
// 냉각 브러시 (도구)
if (brushMode === 'cool') {
    // 온도만 감소, 물질 생성 X
    temp -= 20.0;
    if (temp < -50.0) temp = -50.0;
}
```

**효과:**
- ✅ 물질을 생성하지 않음
- ✅ 브러시 영역의 온도만 감소
- ✅ 열 전도 정상 작동
- ✅ 파우더 토이의 AIR, HEAT 도구와 동일한 방식

---

## 🖌️ 브러시 시스템

### 브러시 모드

#### 1. 물질 모드 (Material)
```javascript
if (brushMode === 'material') {
    wasmModule._addParticle(x, y, selectedType);
}
```
- 선택한 물질 생성
- SAND, WATER, ICE, STEAM, WALL, FIRE, EMPTY

#### 2. 가열 모드 (Heat)
```javascript
if (brushMode === 'heat') {
    temp += 20.0; // 매 프레임 20도 증가
    if (temp > 200.0) temp = 200.0;
}
```
- 온도만 증가
- 최대 200°C

#### 3. 냉각 모드 (Cool)
```javascript
if (brushMode === 'cool') {
    temp -= 20.0; // 매 프레임 20도 감소
    if (temp < -50.0) temp = -50.0;
}
```
- 온도만 감소
- 최소 -50°C

---

### 브러시 크기

**조절 방법:**
- 마우스 휠 위로: 크기 증가
- 마우스 휠 아래로: 크기 감소

**범위:**
- 최소: 1
- 최대: 20
- 기본: 3

**구현:**
```javascript
canvas.addEventListener('wheel', (e) => {
    e.preventDefault();
    if (e.deltaY < 0) {
        brushSize = Math.min(brushSize + 1, MAX_BRUSH_SIZE);
    } else {
        brushSize = Math.max(brushSize - 1, MIN_BRUSH_SIZE);
    }
    updateBrushSizeDisplay();
});
```

---

### 브러시 모양

#### 1. 원형 (Circle)
```javascript
if (brushShape === 'circle') {
    inBrush = (dx * dx + dy * dy <= brushSize * brushSize);
}
```
- 부드러운 원형
- 기본 모양

#### 2. 사각형 (Square)
```javascript
if (brushShape === 'square') {
    inBrush = true; // -brushSize ~ +brushSize 범위 전체
}
```
- 정사각형 영역
- 벽 그리기에 유용

---

## 🎨 UI 구성

### 브러시 컨트롤 패널
```html
<div class="brush-controls">
    <!-- 브러시 모드 -->
    <button class="brush-mode-btn active" data-brush-mode="material">물질</button>
    <button class="brush-mode-btn" data-brush-mode="heat">🔥 가열</button>
    <button class="brush-mode-btn" data-brush-mode="cool">❄️ 냉각</button>
    
    <!-- 브러시 크기 -->
    <span id="brushSizeDisplay">3</span> (마우스 휠로 조절)
    
    <!-- 브러시 모양 -->
    <button class="brush-shape-btn active" data-shape="circle">⭕ 원형</button>
    <button class="brush-shape-btn" data-shape="square">⬜ 사각형</button>
</div>
```

### 물질 선택 버튼
- 🧱 벽 (WALL)
- ⏳ 모래 (SAND)
- 💧 물 (WATER)
- 🧊 얼음 (ICE)
- 💨 증기 (STEAM)
- 🔥 불 (FIRE)
- 🗑️ 지우개 (EMPTY)

**제거된 버튼:**
- ❌ ~~❄️ 냉기 (FROST)~~ → 냉각 브러시로 대체

---

## 🧪 테스트 시나리오

### 테스트 1: 가열 브러시

**절차:**
1. 얼음 추가
2. 브러시 모드를 "가열"로 변경
3. 얼음 위에 마우스 드래그
4. 온도 모드로 전환

**예상 결과:**
- ✅ 얼음이 점진적으로 가열됨 (파랑 → 초록)
- ✅ 0도 도달 시 물로 변함
- ✅ 계속 가열하면 증기로 변함
- ✅ 물질이 생성되지 않음

### 테스트 2: 냉각 브러시

**절차:**
1. 물 추가
2. 브러시 모드를 "냉각"으로 변경
3. 물 위에 마우스 드래그
4. 온도 모드로 전환

**예상 결과:**
- ✅ 물이 점진적으로 냉각됨 (파랑 → 보라)
- ✅ 0도 이하 시 얼음으로 변함
- ✅ 물질이 생성되지 않음

### 테스트 3: 브러시 크기 조절

**절차:**
1. 마우스 휠 위로 스크롤
2. 브러시 크기 확인
3. 마우스 휠 아래로 스크롤
4. 브러시 크기 확인

**예상 결과:**
- ✅ 휠 위로: 크기 증가 (최대 20)
- ✅ 휠 아래로: 크기 감소 (최소 1)
- ✅ 화면에 크기 표시

### 테스트 4: 브러시 모양

**절차:**
1. 원형 브러시로 모래 그리기
2. 사각형 브러시로 벽 그리기

**예상 결과:**
- ✅ 원형: 부드러운 원 모양
- ✅ 사각형: 정사각형 모양

### 테스트 5: FIRE 확산 제한

**절차:**
1. FIRE 여러 개 추가
2. 불이 퍼지는 모습 관찰
3. 2-3초 대기

**예상 결과:**
- ✅ 불이 2-3세대 확산
- ✅ 자연스럽게 소멸
- ✅ 화면 전체를 덮지 않음

---

## 📊 코드 변경 사항

### C++ (백엔드)

#### `src/particle.h`
```cpp
// FROST 제거
enum ParticleType {
  EMPTY = 0,
  WALL = 1,
  SAND = 2,
  WATER = 3,
  ICE = 4,
  STEAM = 5,
  FIRE = 6
  // FROST = 7 제거
};
```

#### `src/material_db.h`
```cpp
// FROST 정의 제거
const Material g_MaterialDB[] = {
    // ... FIRE까지만
};
```

#### `src/simulation.cpp`
```cpp
// FIRE 확산 제한
if (rand() % 3 == 0 && p.life > 10) {
    int newLife = p.life - 5 - rand() % 6;
    if (newLife > 0) {
        // 확산
    }
}

// FROST 관련 코드 모두 제거
```

---

### JavaScript (프론트엔드)

#### `web/main.js`
```javascript
// 브러시 시스템 변수
let brushMode = 'material';
let brushSize = 3;
let brushShape = 'circle';

// 브러시 적용 함수
function applyBrush(x, y) {
    if (brushMode === 'material') {
        wasmModule._addParticle(x, y, selectedType);
    } else if (brushMode === 'heat') {
        temp += 20.0;
    } else if (brushMode === 'cool') {
        temp -= 20.0;
    }
}

// 마우스 휠 이벤트
canvas.addEventListener('wheel', (e) => {
    // 브러시 크기 조절
});
```

#### `web/index.html`
```html
<!-- 브러시 컨트롤 패널 추가 -->
<div class="brush-controls">
    <!-- 브러시 모드, 크기, 모양 -->
</div>

<!-- FROST 버튼 제거 -->
```

---

## 🎯 설계 철학

### 물질 vs 도구 분리

**파우더 토이의 철학:**
- **물질**: 시뮬레이션 대상 (SAND, WATER 등)
- **도구**: 시뮬레이션 조작 (HEAT, COOL, AIR 등)

**이전 (잘못된 구현):**
```
FIRE = 물질 (O) + 가열 도구 (X)
FROST = 물질 (X) + 냉각 도구 (O)
```

**현재 (올바른 구현):**
```
FIRE = 물질 (O)
가열 브러시 = 도구 (O)
냉각 브러시 = 도구 (O)
```

### 온도 시스템

**도구는 온도만 변경:**
- 물질을 생성하지 않음
- 배경(EMPTY)의 온도도 변경 가능
- 열 전도 시스템과 자연스럽게 통합

**물질은 온도를 가짐:**
- FIRE: 150°C (뜨거운 물질)
- ICE: -10°C (차가운 물질)
- 온도에 따라 상태 전이

---

## 🎉 완료된 기능

### 브러시 시스템
- ✅ 3가지 브러시 모드 (물질, 가열, 냉각)
- ✅ 브러시 크기 조절 (1-20)
- ✅ 브러시 모양 (원형, 사각형)
- ✅ 마우스 휠 지원

### FIRE 개선
- ✅ 확산 제한 (부모보다 life 감소)
- ✅ 자연스러운 소멸

### FROST 제거
- ✅ ParticleType에서 제거
- ✅ MaterialDB에서 제거
- ✅ 모든 관련 코드 제거
- ✅ 냉각 브러시로 대체

### UI 개선
- ✅ 브러시 컨트롤 패널
- ✅ 실시간 브러시 크기 표시
- ✅ 직관적인 버튼 디자인

---

## 🚀 다음 단계

### 추가 브러시 도구
1. **기압 브러시** (PLAN_PRESSURE.md 참조)
   - 기압 증가 (AIR)
   - 기압 감소 (VACUUM)

2. **속도 브러시**
   - 입자에 힘 가하기
   - 바람 효과

3. **복제 브러시**
   - 영역 복사/붙여넣기

### 새 물질 추가
1. **리튬** - 물과 반응하여 폭발
2. **기름** - 물보다 가벼움, 가연성
3. **산** - 물질 부식

---

**브러시 시스템 구현 완료! 🎉**
