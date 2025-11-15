# 화학 반응 시스템 문서

## 📋 개요

화학 반응 시스템은 파우더 토이 시뮬레이션에 동적인 물질 간 상호작용을 추가합니다.
모듈식 설계로 팀원들이 독립적으로 새로운 반응을 추가할 수 있습니다.

## 🏗️ 아키텍처

### 디렉토리 구조

```
src/chemistry/
├── reaction_system.h          # 핵심 인터페이스 정의
├── reaction_system.cpp        # 메인 업데이트 루프
├── reaction_registry.h        # 반응 등록 및 관리
├── reaction_registry.cpp      # 레지스트리 구현
└── reactions/                 # 개별 반응 모듈
    ├── combustion.h/cpp       # 연소 반응
    ├── water_metal.h/cpp      # 물-금속 반응
    └── evaporation.h/cpp      # 증발/응축 반응
```

### 핵심 컴포넌트

#### 1. ReactionResult
반응의 결과를 나타내는 구조체:
```cpp
struct ReactionResult {
    bool occurred;              // 반응 발생 여부
    int new_type_center;       // 중심 입자의 새 타입
    int new_type_neighbor;     // 이웃 입자의 새 타입
    float heat_released;       // 방출된 열 (J)
    int explosion_radius;      // 폭발 반경
    float explosion_force;     // 폭발 강도
};
```

#### 2. ReactionRule
반응 규칙을 정의하는 구조체:
```cpp
struct ReactionRule {
    int reactant_a;            // 반응물 A 타입
    int reactant_b;            // 반응물 B 타입
    ReactionFunc handler;      // 반응 처리 함수
    float probability;         // 반응 확률 (0.0~1.0)
    float min_temperature;     // 최소 온도 조건
    const char* name;          // 반응 이름
};
```

#### 3. ReactionRegistry
싱글톤 패턴으로 모든 반응을 관리:
```cpp
ReactionRegistry& registry = ReactionRegistry::getInstance();
registry.registerReaction({...});
```

## 🧪 구현된 화학 반응

### 1. 연소 반응 (combustion.cpp)

#### 나무 + 산소 → 불
- **조건**: 온도 200°C 이상
- **확률**: 30%
- **결과**: 나무 → 불, 산소 소모
- **열 방출**: 15kJ

#### 기름 + 산소 → 불
- **조건**: 온도 150°C 이상
- **확률**: 50%
- **결과**: 기름 → 불, 산소 소모
- **열 방출**: 30kJ (나무보다 강함)

#### 수소 + 산소 → 폭발
- **조건**: 온도 500°C 이상
- **확률**: 70%
- **결과**: 수소 + 산소 → 수증기
- **열 방출**: 50kJ
- **폭발**: 반경 5칸, 강도 3.0

### 2. 물-금속 반응 (water_metal.cpp)

#### 물 + 리튬 → 폭발
- **조건**: 없음 (즉시 반응)
- **확률**: 80%
- **결과**: 물 → 수소, 리튬 → 불
- **열 방출**: 40kJ
- **폭발**: 반경 4칸, 강도 2.5

#### 물 + 나트륨 → 폭발
- **조건**: 없음 (즉시 반응)
- **확률**: 75%
- **결과**: 물 → 수소, 나트륨 → 불
- **열 방출**: 35kJ
- **폭발**: 반경 3칸, 강도 2.0

### 3. 증발/응축 반응 (evaporation.cpp)

#### 기름 + 열 → 유증기
- **조건**: 온도 300°C 이상 또는 불과 접촉
- **확률**: 40%
- **결과**: 기름 → 유증기
- **열 흡수**: -5kJ (흡열 반응)

#### 유증기 → 기름 (응축)
- **조건**: 온도 300°C 이하
- **확률**: 30%
- **결과**: 유증기 → 기름
- **열 방출**: 5kJ (발열 반응)

## 🔧 새로운 반응 추가하기

### Step 1: 반응 파일 생성

```bash
# 헤더 파일
touch src/chemistry/reactions/my_reaction.h

# 구현 파일
touch src/chemistry/reactions/my_reaction.cpp
```

### Step 2: 반응 핸들러 작성

```cpp
// my_reaction.cpp
#include "my_reaction.h"
#include "../reaction_system.h"

// 반응 핸들러 함수
ReactionResult react_my_reaction(
    const Particle& p1, const Particle& p2,
    int x1, int y1, int x2, int y2
) {
    ReactionResult result;
    
    // 조건 체크
    if (p1.temperature < 100.0f) {
        return result; // 반응 안 함
    }
    
    // 확률 체크
    if (randomFloat() > 0.5f) {
        return result;
    }
    
    // 반응 발생!
    result.occurred = true;
    result.new_type_center = NEW_TYPE;
    result.new_type_neighbor = EMPTY;
    result.heat_released = 10000.0f;
    
    return result;
}

// 등록 함수
void registerMyReactions(ReactionRegistry& registry) {
    registry.registerReaction({
        .reactant_a = TYPE_A,
        .reactant_b = TYPE_B,
        .handler = react_my_reaction,
        .probability = 0.5f,
        .min_temperature = 100.0f,
        .name = "My Reaction"
    });
    
    // 양방향 등록 (순서 무관하게 반응)
    registry.registerReaction({
        .reactant_a = TYPE_B,
        .reactant_b = TYPE_A,
        .handler = [](const Particle& b, const Particle& a, 
                      int bx, int by, int ax, int ay) {
            return react_my_reaction(a, b, ax, ay, bx, by);
        },
        .probability = 0.5f,
        .min_temperature = 100.0f,
        .name = "My Reaction (rev)"
    });
}
```

### Step 3: 레지스트리에 등록

```cpp
// reaction_registry.cpp에 추가
#include "reactions/my_reaction.h"

void ReactionRegistry::initializeAllReactions() {
    reactions.clear();
    
    registerCombustionReactions(*this);
    registerWaterMetalReactions(*this);
    registerEvaporationReactions(*this);
    registerMyReactions(*this);  // 추가!
}
```

### Step 4: 빌드 스크립트 업데이트

```bash
# build.sh에 추가
emcc src/simulation.cpp \
    ...
    src/chemistry/reactions/my_reaction.cpp \  # 추가
    -o web/simulation.js \
    ...
```

### Step 5: 빌드 및 테스트

```bash
./build.sh
cd web && python3 -m http.server 8000
```

## 🎯 설계 원칙

### 1. 모듈 독립성
- 각 반응은 독립적인 파일로 관리
- 다른 반응에 영향 없이 수정 가능

### 2. 확장성
- 새로운 반응 추가 시 기존 코드 수정 최소화
- 등록 함수만 호출하면 자동으로 통합

### 3. 협업 친화성
- 여러 팀원이 동시에 다른 반응 작업 가능
- Git 충돌 최소화 (파일 분리)

### 4. 성능
- 해시맵 기반 빠른 반응 조회 (O(1))
- 4방향 이웃만 체크 (8방향 대비 50% 감소)
- 확률 기반 반응으로 CPU 부하 분산

## 📊 성능 분석

### 시간 복잡도
- **반응 체크**: O(WIDTH × HEIGHT × 4) = O(N)
- **반응 조회**: O(1) (평균)
- **전체**: O(N) - 선형 시간

### 메모리 사용
- **ReactionRule**: ~40 bytes/반응
- **등록된 반응**: ~12개 × 40 bytes = 480 bytes
- **무시할 수 있는 수준**

### 예상 오버헤드
- **추가 시간**: 1-2ms/frame
- **60 FPS 유지 가능**

## 🐛 디버깅 팁

### 반응이 발생하지 않을 때

1. **온도 확인**
   ```cpp
   printf("Temp: %.1f°C (min: %.1f°C)\n", 
          p.temperature, rule.min_temperature);
   ```

2. **확률 확인**
   ```cpp
   printf("Probability: %.2f (rolled: %.2f)\n", 
          rule.probability, rand_val);
   ```

3. **반응 등록 확인**
   ```cpp
   printf("Registered reactions: %d\n", 
          registry.getReactionCount());
   ```

### 반응 로깅 추가

```cpp
// reaction_system.cpp의 updateChemistry()에 추가
if (result.occurred) {
    printf("[REACTION] %s at (%d,%d)\n", 
           rule.name, x, y);
}
```

## 🚀 향후 확장 계획

### Phase 2: 추가 반응
- [ ] 산화 반응 (철 + 산소 + 물 → 녹)
- [ ] 중화 반응 (산 + 염기 → 소금 + 물)
- [ ] 중합 반응 (단량체 → 고분자)

### Phase 3: 고급 기능
- [ ] 촉매 시스템 (반응 속도 증가)
- [ ] 연쇄 반응 (한 반응이 다른 반응 유발)
- [ ] 반응 열역학 (엔탈피, 엔트로피)

### Phase 4: 최적화
- [ ] 공간 해싱 (근처 입자만 체크)
- [ ] 반응 캐싱 (같은 조합 재사용)
- [ ] 멀티스레딩 (Worker 활용)

## 📚 참고 자료

- [Powder Toy Wiki](https://powdertoy.co.uk/Wiki/W/Main_Page.html)
- [화학 반응 데이터베이스](https://webbook.nist.gov/)
- [Noita 게임 분석](https://noitagame.com/)

---

**작성일**: 2025-11-15  
**버전**: 1.0.0  
**작성자**: Chemistry System Team
