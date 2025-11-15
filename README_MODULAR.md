# Wasm Powder Toy - Modular Edition

WebAssembly 기반 파우더 토이 시뮬레이터 (모듈식 설계)

## 🎯 특징

- ✅ **모듈식 아키텍처**: 협업 최적화된 구조
- ✅ **물리 시뮬레이션**: 열 전도, 상태 전이, 중력, 밀도
- ✅ **다양한 물질**: SAND, WATER, ICE, STEAM, FIRE
- ✅ **실시간 렌더링**: 물질 타입 / 온도 시각화
- ✅ **브러시 시스템**: 크기, 모양, 모드 조절
- ✅ **FPS 표시**: 성능 모니터링

## 🚀 빠른 시작

### 1. 빌드
```bash
# 모듈식 빌드 (권장)
./build_modular.sh

# 또는 레거시 빌드
./build.sh
```

### 2. 실행
```bash
cd web
python3 -m http.server 8000
```

### 3. 브라우저
```
http://localhost:8000
```

## 📁 프로젝트 구조

```
wasm_powder/
├── src/
│   ├── core/                    # 핵심 시스템
│   │   ├── types.h             # 상수 정의
│   │   ├── grid.h              # 그리드 인터페이스
│   │   └── grid.cpp            # 그리드 구현
│   │
│   ├── physics/                 # 물리 엔진
│   │   ├── heat_conduction.*   # 열 전도
│   │   ├── state_change.*      # 상태 전이
│   │   ├── forces.*            # 중력/부력
│   │   └── movement.*          # 입자 이동
│   │
│   ├── materials/               # 물질 시스템
│   │   ├── material_db.h       # 물질 데이터베이스
│   │   └── special_materials.* # 특수 물질 (FIRE 등)
│   │
│   ├── particle.h              # Particle 구조체
│   ├── simulation.cpp          # (레거시) 단일 파일
│   └── simulation_new.cpp      # (모듈식) 메인 루프
│
├── web/
│   ├── index.html              # UI
│   ├── main.js                 # JavaScript
│   ├── simulation.js           # (빌드 결과)
│   └── simulation.wasm         # (빌드 결과)
│
├── docs/
│   ├── ARCHITECTURE.md         # 아키텍처 문서
│   ├── COLLABORATION_GUIDE.md  # 협업 가이드
│   ├── MODULE_DIAGRAM.md       # 모듈 다이어그램
│   └── plan/                   # 로드맵 및 계획
│
├── build.sh                    # 레거시 빌드
├── build_modular.sh            # 모듈식 빌드
└── REFACTORING_SUMMARY.md      # 리팩토링 요약
```

## 🔧 개발 가이드

### 새로운 물질 추가

1. **물질 정의** (`materials/material_db.h`)
```cpp
enum ParticleType {
  // ...
  OIL = 7
};

Material("Oil", STATE_LIQUID, 900.0f, ...)
```

2. **초기화 로직** (`core/grid.cpp`)
```cpp
case OIL:
  grid[idx].temperature = 20.0f;
  break;
```

3. **UI 추가** (`web/index.html`, `web/main.js`)

### 새로운 물리 패스 추가

1. **모듈 생성** (`physics/viscosity.h/cpp`)
```cpp
void updateViscosity() {
  // 점도 계산 로직
}
```

2. **메인 루프 통합** (`simulation_new.cpp`)
```cpp
updateViscosity();
```

3. **빌드 스크립트** (`build_modular.sh`)
```bash
emcc ... physics/viscosity.cpp ...
```

## 📚 문서

- **[아키텍처 문서](docs/ARCHITECTURE.md)**: 전체 구조 설명
- **[협업 가이드](docs/COLLABORATION_GUIDE.md)**: 팀 작업 방법
- **[모듈 다이어그램](docs/MODULE_DIAGRAM.md)**: 시각적 구조
- **[리팩토링 요약](REFACTORING_SUMMARY.md)**: 변경 사항

## 🎮 사용법

### 브러시 모드
- **Material**: 물질 생성
- **Heat**: 가열 (+20°C)
- **Cool**: 냉각 (-20°C)

### 브러시 크기
- **마우스 휠**: 크기 조절 (1-20)

### 브러시 모양
- **Circle**: 원형
- **Square**: 사각형

### 렌더링 모드
- **물질 타입**: 물질별 색상
- **온도**: 온도별 색상 (파랑→초록→빨강)

## 🧪 테스트 시나리오

### 열 전도
1. FIRE 배치
2. 옆에 ICE 배치
3. ICE가 녹는지 확인

### 상태 전이
1. ICE 배치
2. Heat 브러시로 가열
3. WATER → STEAM 확인

### 밀도
1. WATER 배치
2. 위에 SAND 배치
3. SAND가 가라앉는지 확인

## 🚧 로드맵

### Phase 1: 기본 시스템 ✅
- [x] 모듈식 리팩토링
- [x] 열 전도
- [x] 상태 전이
- [x] 밀도 기반 물리
- [x] 브러시 시스템

### Phase 2: 고급 물리 🚧
- [ ] 잠열 시스템
- [ ] 점도 시스템
- [ ] 화학 반응
- [ ] 기압 시스템

### Phase 3: 최적화 📋
- [ ] 청크 시스템
- [ ] 멀티스레딩
- [ ] WebGL 렌더링

## 🤝 협업

### 브랜치 전략
```
main
├── feature/physics-*
├── feature/material-*
├── feature/ui-*
└── bugfix/*
```

### 커밋 메시지
```
[모듈] 작업 내용

예시:
[physics] Add latent heat system
[materials] Add oil material
[ui] Improve brush controls
```

### PR 체크리스트
- [ ] 빌드 성공
- [ ] 기존 기능 정상 동작
- [ ] 코드 주석 추가
- [ ] 문서 업데이트

## 📊 성능

- **FPS**: 60fps (빈 화면)
- **FPS**: 30-45fps (화면 가득)
- **메모리**: ~33MB
- **WebAssembly**: ~100KB

## 🛠️ 기술 스택

- **언어**: C++11, JavaScript
- **컴파일러**: Emscripten
- **렌더링**: Canvas 2D
- **빌드**: Bash script

## 📝 라이선스

MIT License

## 👥 기여자

환영합니다! `docs/COLLABORATION_GUIDE.md` 참고

## 🐛 버그 리포트

GitHub Issues 사용

---

**모듈식 설계로 협업 가능한 파우더 토이! 🎉**
