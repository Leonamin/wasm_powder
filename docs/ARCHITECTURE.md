# Wasm Powder Toy - 아키텍처 문서

## 개요

이 프로젝트는 **모듈식 설계**를 채택하여 협업과 유지보수를 용이하게 합니다.

## 디렉토리 구조

```
src/
├── core/                    # 핵심 시스템
│   ├── types.h             # 공통 타입 및 상수 정의
│   ├── grid.h/cpp          # 그리드 관리 (메모리, 초기화, 렌더링)
│   └── chunk_manager.*     # (미래) 청크 최적화 시스템
│
├── physics/                 # 물리 시뮬레이션
│   ├── heat_conduction.*   # PASS 2: 열 전도
│   ├── state_change.*      # PASS 3: 상태 전이 (ICE↔WATER↔STEAM)
│   ├── forces.*            # PASS 4: 중력/부력 계산
│   └── movement.*          # PASS 5: 입자 이동 및 교환
│
├── materials/               # 물질 관련
│   ├── material_db.h       # 물질 데이터베이스
│   └── special_materials.* # PASS 4.5: FIRE 등 특수 물질
│
├── particle.h              # Particle 구조체 정의
├── simulation.cpp          # (레거시) 단일 파일 버전
└── simulation_new.cpp      # (모듈식) 메인 루프만 포함
```

## 빌드 시스템

### 레거시 빌드 (단일 파일)
```bash
./build.sh
```
- `src/simulation.cpp` 하나만 컴파일
- 모든 로직이 한 파일에 포함

### 모듈식 빌드 (권장)
```bash
./build_modular.sh
```
- 여러 `.cpp` 파일을 개별적으로 컴파일
- 각 모듈을 독립적으로 수정 가능

## 모듈 설명

### 1. Core 모듈

#### `types.h`
- 전역 상수 정의 (WIDTH, HEIGHT, GRAVITY 등)
- 모든 모듈에서 공유하는 타입

#### `grid.h/cpp`
- **데이터**: `grid[]`, `nextGrid[]`, `renderBuffer[]`, `activeChunks[]`
- **함수**:
  - `initGrid()`: 그리드 초기화
  - `updateRenderBuffer()`: 렌더링 버퍼 업데이트
  - `addParticle()`: 입자 추가
  - `getIndex()`, `inBounds()`: 헬퍼 함수

### 2. Physics 모듈

각 모듈은 **하나의 시뮬레이션 패스**를 담당합니다.

#### `heat_conduction.cpp` (PASS 2)
- 주변 4칸과의 온도 평균 계산
- 비열을 고려한 열 전도율 적용
- 온도 변화 시 청크 활성화

#### `state_change.cpp` (PASS 3)
- 온도에 따른 물질 상태 전이
- ICE → WATER → STEAM (양방향)
- 녹는점/끓는점 체크

#### `forces.cpp` (PASS 4)
- 중력 적용 (밀도 기반)
- 부력 계산 (공기보다 가벼우면 위로)
- 속도 제한

#### `movement.cpp` (PASS 5)
- 입자의 실제 이동 처리
- 상태별 이동 패턴:
  - POWDER: 아래로 떨어짐
  - LIQUID: 아래 + 좌우 확산
  - GAS: 위로 상승 + 확산
- 밀도 기반 교환

### 3. Materials 모듈

#### `material_db.h`
- 각 물질의 물리적 속성 정의
- 밀도, 비열, 녹는점, 끓는점, 점도 등

#### `special_materials.cpp` (PASS 4.5)
- FIRE의 수명 감소
- FIRE의 주변 가열 (온도 증가)
- FIRE의 확산 (뜨거운 곳으로)

### 4. Main 루프

#### `simulation_new.cpp`
- WebAssembly 함수 export
- 시뮬레이션 메인 루프:
  1. 준비 (nextGrid 복사)
  2. 각 패스 순차 실행
  3. 그리드 교체
  4. 렌더 버퍼 업데이트

## 협업 가이드

### 새로운 물질 추가

1. **`material_db.h`** 수정
   ```cpp
   // ParticleType enum에 추가
   enum ParticleType {
     // ...
     OIL = 7
   };
   
   // g_MaterialDB 배열에 추가
   Material("Oil", STATE_LIQUID, 900.0f, ...)
   ```

2. **`grid.cpp`의 `addParticle()`** 수정
   ```cpp
   case OIL:
     grid[idx].temperature = 20.0f;
     grid[idx].life = -1;
     break;
   ```

3. **프론트엔드** 수정
   - `web/index.html`: 버튼 추가
   - `web/main.js`: 색상 추가

### 새로운 물리 패스 추가

1. **`src/physics/`**에 새 파일 생성
   ```cpp
   // chemistry.h
   void updateChemistry();
   
   // chemistry.cpp
   void updateChemistry() {
     // 화학 반응 로직
   }
   ```

2. **`simulation_new.cpp`**에 통합
   ```cpp
   #include "physics/chemistry.h"
   
   void update() {
     // ...
     updateChemistry();  // 새 패스 추가
     updateHeatConduction();
     // ...
   }
   ```

3. **`build_modular.sh`**에 파일 추가
   ```bash
   emcc src/simulation_new.cpp \
       src/physics/chemistry.cpp \  # 추가
       ...
   ```

### 특수 물질 로직 추가

**`materials/special_materials.cpp`** 수정
```cpp
void updateLifeAndSpecialMaterials() {
  // ...
  
  // 새로운 특수 물질
  if (p.type == LAVA) {
    // 용암 로직
  }
}
```

## 장점

### 1. **모듈 독립성**
- 각 파일이 하나의 책임만 가짐
- 다른 모듈에 영향 없이 수정 가능

### 2. **협업 용이성**
- 여러 사람이 동시에 다른 모듈 작업 가능
- Git 충돌 최소화

### 3. **테스트 용이성**
- 각 모듈을 독립적으로 테스트 가능
- 버그 발생 시 범위 좁히기 쉬움

### 4. **확장성**
- 새로운 기능 추가 시 새 파일만 생성
- 기존 코드 수정 최소화

### 5. **가독성**
- 각 파일이 짧고 명확함
- 주석과 함수명으로 의도 파악 쉬움

## 향후 계획

### Phase 3: 고급 기능
- `physics/latent_heat.cpp`: 잠열 시스템
- `physics/viscosity.cpp`: 점도 시스템
- `physics/chemistry.cpp`: 화학 반응
- `physics/pressure.cpp`: 기압 시스템

### Phase 4: 최적화
- `core/chunk_manager.cpp`: 청크 최적화 재도전
- `core/spatial_hash.cpp`: 공간 해싱
- `core/thread_pool.cpp`: 멀티스레딩 (Worker)

### Phase 5: 고급 렌더링
- `rendering/shader.cpp`: WebGL 셰이더
- `rendering/particle_effects.cpp`: 파티클 효과
- `rendering/lighting.cpp`: 동적 조명

## 참고 자료

- [Emscripten 문서](https://emscripten.org/docs/)
- [Powder Toy Wiki](https://powdertoy.co.uk/Wiki/W/Main_Page.html)
- [Cellular Automata](https://en.wikipedia.org/wiki/Cellular_automaton)
