# 구현 완료 요약 (IMPLEMENTATION_SUMMARY.md)

## 📋 구현 개요

파우더 토이 시뮬레이션 엔진을 C++/WebAssembly 기반으로 구현했습니다.

**최종 업데이트:** 2025-12-13

---

## ✅ 현재 구현 상태

### 1. 핵심 시스템

#### A. MaterialDB 시스템 (`src/material_db.h`)

**구현된 구조체:**
```cpp
struct Material {
    const char* name;
    int default_state;             // SOLID, POWDER, LIQUID, GAS
    float density;                 // 밀도 (kg/m³)
    float specific_heat;           // 비열 (미사용)
    float melting_point;           // 녹는점 (미사용)
    float boiling_point;           // 끓는점 (미사용)
    float latent_heat_fusion;      // 융해 잠열 (미사용)
    float latent_heat_vaporization; // 기화 잠열 (미사용)
    float viscosity;               // 점도 (미사용)
    int color[3];                  // RGB
};
```

**정의된 물질 (16개):**

| ID | 물질 | 상태 | 밀도 | 설명 |
|----|------|------|------|------|
| 0 | EMPTY | 기체 | 1.2 | 배경 (공기) |
| 1 | WALL | 고체 | 2500 | 고정 벽 |
| 2 | SAND | 가루 | 1600 | 모래 |
| 3 | WATER | 액체 | 1000 | 물 |
| 4 | ICE | 고체 | 917 | 얼음 |
| 5 | STEAM | 기체 | 0.6 | 수증기 |
| 6 | FIRE | 기체 | 0.3 | 불 (수명 있음) |
| 7 | OXYGEN | 기체 | 1.4 | 산소 |
| 8 | HYDROGEN | 기체 | 0.09 | 수소 |
| 9 | STEAM_OIL | 기체 | 2.5 | 유증기 |
| 10 | WOOD | 고체 | 600 | 나무 |
| 11 | IRON | 고체 | 7874 | 철 |
| 12 | LITHIUM | 가루 | 534 | 리튬 |
| 13 | SODIUM | 가루 | 971 | 나트륨 |
| 14 | OIL | 액체 | 900 | 기름 |
| 15 | CO2 | 기체 | 1.98 | 이산화탄소 |

---

#### B. Particle 구조체 (`src/particle.h`)

```cpp
struct Particle {
    int type;                    // 물질 ID
    float temperature;           // 온도 (미사용)
    int state;                   // 물리 상태 (SOLID, POWDER, LIQUID, GAS)
    float vx, vy;                // 속도
    float latent_heat_storage;   // 잠열 (미사용)
    int life;                    // 수명 (-1 = 무한)
    bool updated_this_frame;     // 이동 플래그
};
```

---

### 2. 시뮬레이션 루프 (`src/simulation.cpp`)

```cpp
void update() {
    // PASS 0: 준비
    memcpy(nextGrid, grid, sizeof(grid));
    
    // PASS 1: 화학 반응 
    updateChemistry();
    
    // PASS 2: 열 전도 
    // updateHeatConduction();
    
    // PASS 3: 상태 전이 
    // updateStateChange();
    
    // PASS 4: 힘 계산 
    updateForces();
    
    // PASS 4.5: 수명 및 특수 물질 
    updateLifeAndSpecialMaterials();
    
    // PASS 5: 이동 
    updateMovement();
    
    // FINAL: 그리드 교체
    memcpy(grid, nextGrid, sizeof(grid));
}
```

---

### 3. 물리 시스템

#### A. 힘 계산 (`src/physics/forces.cpp`) 

- 밀도 기반 중력/부력 계산
- 공기(1.2 kg/m³) 기준으로 위/아래 방향 결정

#### B. 이동 시스템 (`src/physics/movement.cpp`) 

**상태별 이동 패턴:**

| 상태 | 이동 방향 | 확산 거리 | 특징 |
|------|----------|----------|------|
| POWDER | 아래 → 대각선 | - | 랜덤 좌우 선택 |
| LIQUID | 아래 → 대각선 → 수평 | 10칸 | 빠른 수평 확산 |
| GAS | 위(70%) → 수평(30%) | 5칸 | 랜덤 확산 |
| FIRE | 위 → 대각선 → 수평 | 3칸 | 특수 처리 |

**핵심 기능:**
-  밀도 기반 교환 (무거운 물질이 가라앉음)
-  랜덤 방향 선택 (자연스러운 움직임)
-  고체는 밀어낼 수 없음

#### C. 열 전도 (`src/physics/heat_conduction.cpp`) 

#### D. 상태 전이 (`src/physics/state_change.cpp`) 

---

### 4. 화학 반응 시스템 

#### 구조 (`src/chemistry/`)

```
chemistry/
├── reaction_system.cpp/h      # 반응 처리 메인 루프
├── reaction_registry.cpp/h    # 반응 규칙 등록/관리
└── reactions/
    ├── combustion.cpp/h       # 연소 반응
    ├── water_metal.cpp/h      # 물-금속 반응
    └── evaporation.cpp/h      # 증발/응축 반응
```

#### 구현된 반응

**연소 반응 (combustion.cpp):**
| 반응물 | 생성물 | 확률 | 설명 |
|--------|--------|------|------|
| WOOD + FIRE | FIRE + CO2 | 50% | 나무 연소 |
| OIL + FIRE | FIRE + CO2 | 70% | 기름 연소 |
| HYDROGEN + FIRE | FIRE + STEAM | 80% | 수소 폭발 |
| ICE + FIRE | WATER + STEAM | 100% | 얼음 녹음 |

**물-금속 반응 (water_metal.cpp):**
| 반응물 | 생성물 | 확률 | 설명 |
|--------|--------|------|------|
| WATER + LITHIUM | HYDROGEN + FIRE | 80% | 리튬 폭발 |
| WATER + SODIUM | HYDROGEN + FIRE | 75% | 나트륨 폭발 |

---

### 5. 특수 물질 시스템 (`src/materials/special_materials.cpp`)

**FIRE 특수 처리:**
- 수명(life) 시스템: 시간이 지나면 소멸
- 주변 가열 기능 (현재 비활성화)

---

## 현재 활성화된 기능 요약

| 기능 | 상태 | 파일 |
|------|------|------|
| 화학 반응 |  활성화 | `chemistry/` |
| 힘 계산 (중력/부력) |  활성화 | `physics/forces.cpp` |
| 이동 시스템 |  활성화 | `physics/movement.cpp` |
| 수명 시스템 |  활성화 | `materials/special_materials.cpp` |
| 열 전도 |  비활성화 | `physics/heat_conduction.cpp` |
| 상태 전이 (온도 기반) |  비활성화 | `physics/state_change.cpp` |
| 온도 감쇠 |  비활성화 | - |

---

## 사용 가능한 물질

### 기본 물질
- **WALL** - 고정 벽
- **SAND** - 모래 (가루)
- **WATER** - 물 (액체)
- **ICE** - 얼음 (고체)
- **STEAM** - 수증기 (기체)
- **FIRE** - 불 (수명 있음)

### 연소 가능 물질
- **WOOD** - 나무 (불에 탐)
- **OIL** - 기름 (불에 잘 탐)
- **HYDROGEN** - 수소 (폭발)

### 반응성 금속
- **LITHIUM** - 리튬 (물과 폭발)
- **SODIUM** - 나트륨 (물과 폭발)

### 기체
- **OXYGEN** - 산소
- **CO2** - 이산화탄소 (무거운 기체)

---

## 기술 스택

- **언어:** C++ (시뮬레이션), JavaScript (프론트엔드)
- **컴파일:** Emscripten → WebAssembly
- **그리드:** 400 x 300 = 120,000 셀
- **렌더링:** Zero-copy (WASM 메모리 직접 접근)

---

## 비활성화된 열 관련 기능

다음 기능들은 코드에 존재하지만 현재 비활성화되어 있습니다:

1. **열 전도** (`updateHeatConduction()`)
2. **상태 전이** (`updateStateChange()`) - 온도 기반 ICE↔WATER↔STEAM
3. **온도 감쇠** (`applyCooling()`)
4. **FIRE 주변 가열** (special_materials.cpp 내)

이 기능들은 `simulation.cpp`에서 주석 처리되어 있으며, 필요시 다시 활성화할 수 있습니다.
