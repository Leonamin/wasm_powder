## 1\. 프로젝트 기획서 (Wasm Environmental Engine)

### 가. 프로젝트 비전

"물질 속성 DB"와 **"병렬 환경 그리드"** 개념을 기반으로 하는 물리/화학 시뮬레이션 엔진을 C++(Wasm)으로 구축한다. 이 엔진은 `EMPTY`(빈 공간) 셀을 포함한 **모든 그리드 셀**이 독립적인 온도와 (미래의) 기압 값을 가지며, 이를 통해 입자-입자 상호작용뿐만 아니라 입자-환경 간의 상호작용(열 전도, 상태 변화)을 시뮬레이션한다.

### 나. 핵심 아키텍처

#### 1\) `Grid`: 병렬 그리드의 통합

  * `Particle grid[WIDTH * HEIGHT];`
  * 이것은 단순히 '입자 목록'이 아니라, **시뮬레이션 공간 그 자체**입니다.
  * `grid[x][y]`는 해당 좌표의 **모든 물리 상태**를 담습니다.
      * `grid[x][y].type` = **물질 그리드** (이 칸에 뭐가 있는가? `SAND`? `EMPTY`?)
      * `grid[x][y].temperature` = **온도 그리드** (이 칸은 몇 도인가? `EMPTY`라도 20°C)
      * `grid[x][y].pressure` = **기압 그리드** (이 칸은 몇 기압인가? [Phase 3])

#### 2\) `Particle` (Cell State): "셀 상태" 구조체 🧬

그리드의 각 셀에 저장될 '현재 상태' 정보입니다.

```cpp
struct Particle { // 'Cell' 또는 'PixelState'로 봐도 무방
    // --- 1. 물질 그리드 ---
    int type; // 물질 ID (0=EMPTY, 1=WALL, 2=SAND...)

    // --- 2. 환경 그리드 (배경 속성) ---
    float temperature; // 현재 셀의 온도 (EMPTY라도 값을 가짐)
    // float pressure; // [Phase 3] 현재 셀의 기압

    // --- 3. 물리 상태 (물질이 있을 경우) ---
    int state; // 물리 상태 (0=SOLID, 1=POWDER, 2=LIQUID, 3=GAS)
    float vx, vy; // 속도
    float latent_heat_storage; // 축적된 잠열

    // --- 4. 기타 상태 ---
    int life; // 수명
    bool updated_this_frame; // 이동 최적화용 플래그
};
```

#### 3\) `MaterialDB` (물질 속성 DB): "물질의 본질" (불변) 📜

`Particle.type`을 키로 조회하는 불변 속성 테이블입니다. (이전 기획서와 동일)

  * **`Material` 구조체:** `name`, `density`, `viscosity`, `specific_heat`, `melting_point`, `boiling_point`, `latent_heat_fusion` (잠열), `default_color` 등.

#### 4\) `ChemistryDB` (화학 반응 테이블) & `Active Chunks` (최적화)

  * (이전 기획서와 동일하게 유지)

-----

### 다. `update()` 루프 (다중 패스 엔진) ⚙️

모든 '활성 청크' 내의 **모든 셀**(`Particle`)에 대해 단계별 물리 법칙을 적용합니다.

```cpp
// Wasm (C++)
void update() {
    // 0. 준비: nextGrid 복사, activeChunks 플래그 초기화
    // ...

    // === PASS 1: 화학 반응 & 생명주기 ===
    // (활성 청크 순회)
    // 1. (생명주기): p.type != EMPTY 이고 p.life > 0 이면 p.life--
    // 2. (수명 종료): p.life == 1 이면 nextGrid[i]를 SMOKE 등으로 변경.
    // 3. (화학 반응): p와 주변 셀을 g_ChemistryDB와 대조하여 반응 실행.

    // === PASS 2: 환경 물리 (열 전도) ===
    // (활성 청크 순회)
    // *** '배경'의 핵심 ***
    // 1. p (EMPTY 포함)와 주변 4칸 셀의 온도를 읽음.
    // 2. MaterialDB[p.type].specific_heat (비열)을 고려하여 온도를 평균.
    //    (EMPTY 셀은 '공기'의 비열 값을 사용)
    // 3. nextGrid[i].temperature에 새 온도 저장.
    //    -> 이것만으로도 EMPTY 공간을 통해 열이 퍼져나감.

    // === PASS 3: 상태 전이 (State Change) ===
    // (활성 청크 순회)
    // 1. p.type != EMPTY 인 입자들에 대해...
    // 2. (잠열 축적): p.temp가 MaterialDB[p.type].melting_point에 도달하면,
    //    nextGrid[i].latent_heat_storage에 'PASS 2'에서 계산된 열 에너지 차이만큼 축적.
    // 3. (상태 변경): latent_heat_storage가 잠열 값을 초과하면,
    //    nextGrid[i].type = (예: ICE -> WATER), state = LIQUID 변경.

    // === PASS 4: 힘 계산 (Forces) ===
    // (활성 청크 순회)
    // 1. (중력/부력): p.state가 POWDER, LIQUID, GAS일 경우 
    //    MaterialDB[p.type].density를 고려하여 nextGrid[i].vy에 힘 가산.

    // === PASS 5: 이동 및 교환 (Movement) ===
    // (활성 청크 순회)
    // 1. p.state가 SOLID가 아닌 입자들에 대해...
    // 2. (밀도 비교): (vx, vy) 기반 목표 셀(target)과 밀도 비교 후 교환(Swap).
    // 3. (점도 기반 퍼짐): 막혔을 경우, MaterialDB[p.type].viscosity(점도)에 따라 
    //    좌우로 퍼지는 로직 (랜덤 요소 포함).
    // *** 중요 ***: 입자가 (x,y)에서 (tx,ty)로 이동할 때,
    // 자신의 '배경 속성'(temperature)을 *함께 가지고 이동*해야 합니다.
    // (예: 1000도의 SAND가 20도의 EMPTY로 이동하면, 
    //      nextGrid[tx,ty].temp = 1000, nextGrid[x,y].temp = 20)

    // === FINAL: 그리드 교체 ===
    memcpy(grid, nextGrid, sizeof(grid));
}
```

-----

## 2\. 단계별 구현 계획 (Roadmap)

'배경 속성' 개념을 검증하는 방향으로 단계를 조정합니다.

### STEP 1: C++ 기반 구축 (Wasm-JS 연동 X)

  * **담당:** 서버 개발자 (C++)
  * **목표:** 핵심 데이터 구조(`Particle`, `MaterialDB`) 정의.
  * **할 일:**
    1.  `Particle` 구조체 정의 (type, temperature 포함).
    2.  `MaterialDB` 구축 (3개: `EMPTY` (공기), `WALL`, `SAND`).
          * `EMPTY`의 `specific_heat` (비열)도 정의해야 함.
    3.  `Grid` (`grid`, `nextGrid`) 생성.
    <!-- end list -->
      * **검증:** C++ `main()`에서 `grid[10][10].temperature = 100`로 설정하고, 이것이 `grid`에 잘 반영되는지 확인.

### STEP 2: 기본 연동 및 **'배경 렌더링'**

  * **담당:** 앱 개발자(JS), 서버 개발자(C++)
  * **목표:** JS가 Wasm의 `Particle` 배열을 읽고, **'온도'를 시각화**합니다.
  * **할 일:**
    1.  (C++) `getParticleArrayPtr()` 함수, `addParticle(x, y, type)` 함수 노출.
    2.  (JS) Wasm 모듈 로드. `Particle` 배열 전체를 JS로 매핑.
    3.  (JS) **렌더링 모드 선택 UI** 생성 (예: '물질 타입 보기', '온도 보기').
    4.  (JS) `gameLoop()`에서 '온도 보기' 모드일 때, `p.temperature` 값을 읽어 **흑백 또는 HSL(색상) 스케일로 캔버스를 렌더링**. (예: 0°C=파랑, 100°C=빨강)
    <!-- end list -->
      * **결과:** 빈 화면(모두 20°C)이 보이고, 마우스로 `FIRE`(`addParticle`)를 추가하면 그 점이 빨갛게 보입니다. (움직임 X)

### STEP 3: "열 전도" 구현 (Pass 2)

  * **담당:** 서버 개발자 (C++)
  * **목표:** '배경' 개념의 핵심인 열 전도를 구현.
  * **할 일:**
    1.  `update()` 함수에 **`PASS 2 (열 전도)`** 로직을 구현. (비열 `specific_heat` 고려)
    2.  (C++) `Active Chunks`의 *기초 버전* 구현 (온도나 타입이 변한 셀의 청크를 `active`로 마킹). `update()`는 `active` 청크만 순회.
    <!-- end list -->
      * **결과:** (JS '온도 보기' 모드에서) `FIRE` 입자 하나를 찍으면, `EMPTY` 셀(배경)을 통해 열이 서서히 퍼져나가는 모습이 시각적으로 보입니다. `FIRE`를 지워도 열은 그 자리에 남아 서서히 식어갑니다.
    3.  새로운 물질, `액체 질소`, `기체 질소`, `고체 질소` 추가. (실제 질소의 비열을 사용하여 액체 질소와 액체 물의 열 전도를 구현)

### STEP 4: "밀도" & "이동" 구현 (Pass 4, 5)

  * **담당:** 서버 개발자 (C++)
  * **목표:** 밀도 기반 교환과 중력을 구현.
  * **할 일:**
    1.  `MaterialDB`에 `WATER` (`density: 1.0`), `SAND` (`density: 1.6`) 속성 정의.
    2.  `update()` 함수에 \*\*`PASS 4 (힘)`\*\*와 **`PASS 5 (이동)`** 로직 구현.
    3.  입자가 이동할 때 자신의 `temperature`를 가지고 이동하도록 로직 수정.
    <!-- end list -->
      * **결과:** (JS '물질 타입 보기' 모드에서) `SAND`가 `WATER`에 가라앉습니다. (JS '온도 보기' 모드에서) 뜨거운 `SAND`가 차가운 `WATER`에 가라앉으며 물을 데우는 것이 보입니다.

### STEP 5: 성능 비교 및 "상태 전이"

  * **담당:** 비개발자, 서버 개발자
  * **목표:** Wasm의 우월성을 증명하고, 시뮬레이션의 완성도를 높입니다.
  * **할 일:**
    1.  **(비개발자)** STEP 3(열 전도)과 STEP 4(이동) 로직을 **순수 JS로 똑같이 구현**합니다.
    2.  **(비개발자)** 500x500 그리드에서 열 전도+입자 이동 시 FPS를 비교/측정하여 PPT 작성.
    3.  **(서버 개발자)** `MaterialDB`에 `ICE` 추가. `update()`에 **`PASS 3 (상태 전이)`** 로직 구현. (일단 잠열 없이, `temp > 0`이면 `WATER`로)
    <!-- end list -->
      * **결과:** JS 대비 압도적인 Wasm 성능 그래프. `FIRE`로 `ICE`를 녹여 `WATER`로 만들고, 이 `WATER`가 `SAND`와 섞이는 복합적인 시뮬레이션 완성.

-----

### STEP 6 (확장): 고급 물리 & 화학

  * **담당:** 서버 개발자 (C++)
  * **목표:** 잠열, 점도, 화학 반응을 구현합니다.
  * **할 일:**
    1.  `PASS 3 (상태 전이)`에 **잠열(`latent_heat_storage`)** 로직을 추가.
    2.  `PASS 5 (이동)`에 \*\*점도(`viscosity`)\*\*를 이용한 액체 퍼짐 로직 추가.
    3.  `ChemistryDB`와 \*\*`PASS 1 (화학)`\*\*을 구현.
    <!-- end list -->
      * **결과:** 얼음이 0°C에서 바로 녹지 않고 한동안 '열을 흡수'하는 현상이 구현됩니다.

### STEP 7 (확장): 기압
  * **담당:** 서버 개발자 (C++)
  * **목표:** 기압을 구현합니다.
  * **할 일:**
    1.  `MaterialDB`에 `AIR` 추가. `update()`에 \*\*`PASS 2 (기압)`\*\* 로직 구현.
    2.  파우더 토이처럼 0은 진공 1은 표준 대기압을 의미합니다. 그 이상으로 갈 경우 고기압이며 현재 버전은 1바이트를 사용하여 255까지 표현할 수 있습니다.
    <!-- end list -->
      * **결과:** 기압이 변할 때 입자 이동이 제어됩니다.
