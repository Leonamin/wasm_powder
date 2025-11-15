# 모듈 다이어그램

## 전체 구조

```
┌─────────────────────────────────────────────────────────────┐
│                     WebAssembly Module                       │
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │           simulation_new.cpp (Main Loop)           │    │
│  │                                                     │    │
│  │  init() → update() → export functions              │    │
│  └─────────────────┬───────────────────────────────────┘    │
│                    │                                         │
│  ┌─────────────────┴───────────────────────────────────┐    │
│  │                  Update Cycle                       │    │
│  │                                                     │    │
│  │  1. Prepare (copy grid)                            │    │
│  │  2. Heat Conduction    ──→  physics/               │    │
│  │  3. State Change       ──→  physics/               │    │
│  │  4. Forces             ──→  physics/               │    │
│  │  5. Special Materials  ──→  materials/             │    │
│  │  6. Movement           ──→  physics/               │    │
│  │  7. Finalize (swap grid)                           │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
         │                    │                    │
         ▼                    ▼                    ▼
    ┌────────┐          ┌──────────┐        ┌──────────┐
    │  Core  │          │ Physics  │        │Materials │
    └────────┘          └──────────┘        └──────────┘
```

## 모듈 의존성

```
simulation_new.cpp
    │
    ├─→ core/grid.h
    │       ├─→ core/types.h
    │       └─→ particle.h
    │
    ├─→ physics/heat_conduction.h
    │       ├─→ core/grid.h
    │       ├─→ core/types.h
    │       └─→ material_db.h
    │
    ├─→ physics/state_change.h
    │       ├─→ core/grid.h
    │       ├─→ core/types.h
    │       └─→ material_db.h
    │
    ├─→ physics/forces.h
    │       ├─→ core/grid.h
    │       ├─→ core/types.h
    │       └─→ material_db.h
    │
    ├─→ physics/movement.h
    │       ├─→ core/grid.h
    │       ├─→ core/types.h
    │       └─→ material_db.h
    │
    └─→ materials/special_materials.h
            ├─→ core/grid.h
            ├─→ core/types.h
            └─→ particle.h
```

## 데이터 흐름

```
┌──────────────────────────────────────────────────────────┐
│                    JavaScript (Web)                       │
│                                                           │
│  User Input → addParticle() → WebAssembly                │
│  Canvas ← getRenderBufferPtr() ← WebAssembly             │
└──────────────────┬───────────────────────────────────────┘
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│                  WebAssembly (C++)                        │
│                                                           │
│  ┌─────────────────────────────────────────────┐         │
│  │              Core (grid.cpp)                │         │
│  │                                              │         │
│  │  grid[GRID_SIZE]        ← Current state     │         │
│  │  nextGrid[GRID_SIZE]    ← Next state        │         │
│  │  renderBuffer[GRID_SIZE] ← For rendering    │         │
│  │  activeChunks[CHUNK_COUNT] ← Optimization   │         │
│  └─────────────────────────────────────────────┘         │
│                   │                                       │
│                   ▼                                       │
│  ┌─────────────────────────────────────────────┐         │
│  │         Physics Passes (순차 실행)          │         │
│  │                                              │         │
│  │  [Heat] → [State] → [Force] → [Life] → [Move]│        │
│  │                                              │         │
│  │  각 패스는 nextGrid를 수정                  │         │
│  └─────────────────────────────────────────────┘         │
│                   │                                       │
│                   ▼                                       │
│  ┌─────────────────────────────────────────────┐         │
│  │         Finalize (grid swap)                │         │
│  │                                              │         │
│  │  memcpy(grid, nextGrid, ...)                │         │
│  │  updateRenderBuffer()                       │         │
│  └─────────────────────────────────────────────┘         │
└──────────────────────────────────────────────────────────┘
```

## 각 모듈의 역할

### Core 모듈
```
┌─────────────────────────────────┐
│         core/types.h            │
│                                 │
│  - WIDTH, HEIGHT                │
│  - GRAVITY, VELOCITY_DAMPING    │
│  - HEAT_CONDUCTION_BASE         │
└─────────────────────────────────┘
         │ include
         ▼
┌─────────────────────────────────┐
│         core/grid.h/cpp         │
│                                 │
│  Data:                          │
│  - grid[]                       │
│  - nextGrid[]                   │
│  - renderBuffer[]               │
│  - activeChunks[]               │
│                                 │
│  Functions:                     │
│  - initGrid()                   │
│  - addParticle()                │
│  - updateRenderBuffer()         │
│  - getIndex(), inBounds()       │
│  - markChunkActive()            │
└─────────────────────────────────┘
```

### Physics 모듈
```
┌──────────────────────────────────────────────────────────┐
│                   physics/                                │
│                                                           │
│  ┌─────────────────────┐  ┌─────────────────────┐       │
│  │ heat_conduction.cpp │  │  state_change.cpp   │       │
│  │                     │  │                     │       │
│  │ - 주변 온도 평균    │  │ - ICE↔WATER↔STEAM  │       │
│  │ - 비열 고려         │  │ - 녹는점/끓는점    │       │
│  └─────────────────────┘  └─────────────────────┘       │
│                                                           │
│  ┌─────────────────────┐  ┌─────────────────────┐       │
│  │     forces.cpp      │  │    movement.cpp     │       │
│  │                     │  │                     │       │
│  │ - 중력 계산         │  │ - POWDER: 아래로    │       │
│  │ - 부력 계산         │  │ - LIQUID: 확산      │       │
│  │ - 속도 제한         │  │ - GAS: 위로         │       │
│  └─────────────────────┘  └─────────────────────┘       │
└──────────────────────────────────────────────────────────┘
```

### Materials 모듈
```
┌─────────────────────────────────┐
│      materials/material_db.h    │
│                                 │
│  Material {                     │
│    name, density, specific_heat │
│    melting_point, boiling_point │
│    viscosity, color             │
│  }                              │
│                                 │
│  g_MaterialDB[] = {             │
│    EMPTY, WALL, SAND,           │
│    WATER, ICE, STEAM, FIRE      │
│  }                              │
└─────────────────────────────────┘
         │ use
         ▼
┌─────────────────────────────────┐
│  materials/special_materials.cpp│
│                                 │
│  - FIRE 수명 감소               │
│  - FIRE 주변 가열               │
│  - FIRE 확산 (뜨거운 곳으로)    │
│  - (미래) 화학 반응             │
└─────────────────────────────────┘
```

## 시뮬레이션 패스 순서

```
Frame N
  │
  ├─ PASS 0: Prepare
  │    └─ memcpy(nextGrid, grid)
  │
  ├─ PASS 1: Chemistry (미구현)
  │    └─ 화학 반응 처리
  │
  ├─ PASS 2: Heat Conduction
  │    └─ 열 전도 계산
  │
  ├─ PASS 3: State Change
  │    └─ 상태 전이 (ICE↔WATER↔STEAM)
  │
  ├─ PASS 4: Forces
  │    └─ 중력/부력 계산
  │
  ├─ PASS 4.5: Special Materials
  │    └─ FIRE 수명, 확산, 가열
  │
  ├─ PASS 5: Movement
  │    └─ 입자 이동 및 교환
  │
  └─ FINAL: Finalize
       ├─ memcpy(grid, nextGrid)
       └─ updateRenderBuffer()
  │
Frame N+1
```

## 협업 시나리오

### 시나리오 1: 새로운 물질 추가 (OIL)

```
Developer A (Materials)
  │
  ├─ materials/material_db.h
  │    └─ Add OIL enum and properties
  │
  ├─ core/grid.cpp
  │    └─ Add OIL case in addParticle()
  │
  └─ (Optional) materials/special_materials.cpp
       └─ Add OIL-specific behavior

Developer B (Frontend)
  │
  ├─ web/index.html
  │    └─ Add OIL button
  │
  └─ web/main.js
       └─ Add OIL color

→ No conflicts! 다른 파일 수정
```

### 시나리오 2: 새로운 물리 법칙 (점도)

```
Developer C (Physics)
  │
  ├─ physics/viscosity.h/cpp
  │    └─ Create new module
  │
  ├─ simulation_new.cpp
  │    └─ Add updateViscosity() call
  │
  └─ build_modular.sh
       └─ Add viscosity.cpp to compile

→ 기존 physics 모듈 영향 없음
```

### 시나리오 3: UI 개선

```
Developer D (Frontend)
  │
  ├─ web/index.html
  │    └─ Redesign UI
  │
  └─ web/main.js
       └─ Add new features

→ C++ 코드 수정 불필요
```

## 빌드 프로세스

```
build_modular.sh
  │
  ├─ Compile: simulation_new.cpp
  ├─ Compile: core/grid.cpp
  ├─ Compile: physics/heat_conduction.cpp
  ├─ Compile: physics/state_change.cpp
  ├─ Compile: physics/forces.cpp
  ├─ Compile: physics/movement.cpp
  ├─ Compile: materials/special_materials.cpp
  │
  ├─ Link: All object files
  │
  └─ Output: web/simulation.js + web/simulation.wasm
```
