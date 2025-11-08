# 협업 가이드

## 빠른 시작

### 1. 프로젝트 클론
```bash
git clone <repository-url>
cd wasm_powder
```

### 2. 빌드
```bash
# 모듈식 빌드 (권장)
./build_modular.sh

# 또는 레거시 빌드
./build.sh
```

### 3. 실행
```bash
cd web
python3 -m http.server 8000
# http://localhost:8000 접속
```

## 작업 영역 분리

### 물리 엔진 개발자
**담당 영역**: `src/physics/`

**작업 예시**:
- 새로운 물리 법칙 추가
- 기존 패스 성능 개선
- 버그 수정

**파일**:
- `heat_conduction.cpp`: 열 전도 로직
- `state_change.cpp`: 상태 전이 로직
- `forces.cpp`: 중력/부력 계산
- `movement.cpp`: 입자 이동

**주의사항**:
- `grid.h`의 헬퍼 함수 사용
- `types.h`의 상수 사용
- 다른 패스에 영향 최소화

### 물질 개발자
**담당 영역**: `src/materials/`

**작업 예시**:
- 새로운 물질 추가 (기름, 용암, 산 등)
- 특수 물질 동작 구현
- 물질 간 상호작용

**파일**:
- `material_db.h`: 물질 속성 정의
- `special_materials.cpp`: FIRE 등 특수 로직

**체크리스트**:
1. `material_db.h`에 enum 추가
2. `g_MaterialDB` 배열에 속성 추가
3. `grid.cpp`의 `addParticle()` 수정
4. `special_materials.cpp`에 로직 추가 (필요 시)
5. 프론트엔드 UI 추가

### 프론트엔드 개발자
**담당 영역**: `web/`

**작업 예시**:
- UI/UX 개선
- 새로운 브러시 도구
- 렌더링 모드 추가

**파일**:
- `index.html`: UI 구조
- `main.js`: JavaScript 로직

**주의사항**:
- C++ 코드 수정 불필요
- WebAssembly 함수 호출만 사용

### 코어 시스템 개발자
**담당 영역**: `src/core/`

**작업 예시**:
- 그리드 메모리 최적화
- 청크 시스템 개선
- 새로운 자료구조 추가

**파일**:
- `grid.cpp`: 그리드 관리
- `types.h`: 전역 상수
- `chunk_manager.cpp`: (미래) 청크 최적화

**주의사항**:
- 모든 모듈에 영향 가능
- 충분한 테스트 필요
- 팀원과 사전 논의

## Git 워크플로우

### 브랜치 전략
```
main
├── feature/physics-latent-heat
├── feature/material-oil
├── feature/ui-brush-tools
└── bugfix/fire-spread
```

### 커밋 메시지 규칙
```
[모듈] 작업 내용

예시:
[physics] Add latent heat system
[materials] Add oil material
[ui] Improve brush size control
[core] Fix grid memory leak
```

### PR 체크리스트
- [ ] 빌드 성공 (`./build_modular.sh`)
- [ ] 기존 기능 정상 동작
- [ ] 코드 주석 추가
- [ ] 관련 문서 업데이트
- [ ] 충돌 해결

## 코딩 스타일

### C++
```cpp
// 함수명: camelCase
void updateHeatConduction() { }

// 변수명: camelCase
float newTemp = 0.0f;

// 상수: UPPER_CASE
const float GRAVITY = 0.3f;

// 주석: 한글 OK
// 열 전도율 계산
```

### JavaScript
```javascript
// 함수명: camelCase
function updateFPSDisplay() { }

// 변수명: camelCase
let brushSize = 3;

// 상수: UPPER_CASE
const MAX_BRUSH_SIZE = 20;
```

## 테스트 시나리오

### 물리 엔진 테스트
1. **열 전도**: FIRE 옆에 ICE 배치 → 녹아야 함
2. **상태 전이**: ICE 가열 → WATER → STEAM
3. **밀도**: SAND를 WATER 위에 → 가라앉아야 함
4. **부력**: STEAM이 위로 올라가는지

### 물질 테스트
1. **새 물질 추가**: 버튼 클릭 → 생성 확인
2. **특수 동작**: FIRE 확산, 수명 확인
3. **상호작용**: 물질 간 반응 확인

### UI 테스트
1. **브러시**: 크기 조절, 모양 변경
2. **렌더링**: 물질/온도 모드 전환
3. **FPS**: 표시 정확도

## 문제 해결

### 빌드 실패
```bash
# Emscripten 확인
emcc --version

# 경로 확인
ls src/physics/
ls src/materials/

# 클린 빌드
rm -rf web/simulation.*
./build_modular.sh
```

### 함수 not found
- `build_modular.sh`의 EXPORTED_FUNCTIONS 확인
- `simulation_new.cpp`의 extern "C" 확인

### 성능 저하
- FPS 표시 확인
- 브라우저 콘솔 확인
- 청크 시스템 비활성화 확인

## 리소스

- **아키텍처 문서**: `docs/ARCHITECTURE.md`
- **로드맵**: `docs/plan/PLAN_ROADMAP.md`
- **버그 트래킹**: `docs/plan/PLAN_BUG.md`

## 질문/토론

- GitHub Issues 사용
- 모듈별 담당자에게 멘션
- 주간 회의에서 논의
