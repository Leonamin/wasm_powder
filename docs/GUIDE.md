# Wasm 파우더 토이 - 구현 가이드

## 📁 프로젝트 구조

```
wasm_powder/
├── src/
│   ├── particle.h          # 입자 구조체 정의
│   └── simulation.cpp      # C++ 시뮬레이션 로직
├── web/
│   ├── index.html          # 메인 HTML 인터페이스
│   ├── main.js             # JavaScript 렌더링/UI
│   ├── simulation.js       # (빌드 후 생성)
│   └── simulation.wasm     # (빌드 후 생성)
├── build.sh                # Wasm 빌드 스크립트
├── GUIDE.md                # 이 파일
└── README.md               # 프로젝트 기획서
```

## 🚀 빠른 시작

### 1. 필수 도구 설치

**Emscripten** (C++를 WebAssembly로 컴파일)

```bash
# macOS
brew install emscripten

# Linux
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### 2. 프로젝트 빌드

```bash
# 빌드 스크립트 실행
./build.sh
```

성공하면 `web/` 디렉토리에 `simulation.js`와 `simulation.wasm` 파일이 생성됩니다.

### 3. 개발 서버 실행

```bash
# web 디렉토리로 이동
cd web

# Python 내장 서버 실행 (포트 8000)
python3 -m http.server 8000

# 또는 Node.js http-server 사용
# npm install -g http-server
# http-server -p 8000
```

### 4. 브라우저에서 열기

http://localhost:8000 접속

## 🎮 사용법

1. **입자 선택**: 화면의 버튼을 클릭하여 입자 타입 선택
2. **그리기**: 캔버스에 마우스 드래그로 입자 그리기
3. **관찰**: 입자들의 물리 시뮬레이션 관찰
4. **지우기**: "전체 지우기" 버튼으로 초기화

## 🧪 구현된 입자 (Phase 1)

| 입자            | 타입      | 설명                                  |
| --------------- | --------- | ------------------------------------- |
| 🧱 벽 (WALL)    | 고정 고체 | 움직이지 않음                         |
| ⏳ 모래 (SAND)  | 가루 고체 | 중력으로 아래로 떨어짐                |
| 💧 물 (WATER)   | 액체      | 아래로 떨어지고 옆으로 퍼짐           |
| 🧊 얼음 (ICE)   | 고정 고체 | 0°C 이하에서 물이 변함                |
| 💨 증기 (STEAM) | 기체      | 100°C 이상에서 물이 변함, 위로 올라감 |
| 🔥 불 (FIRE)    | 온도원    | 주변을 150°C로 가열                   |
| ❄️ 냉기 (FROST) | 온도원    | 주변을 -20°C로 냉각                   |

## 🔧 코드 구조

### C++ (src/simulation.cpp)

**핵심 함수:**

- `init()`: 그리드 초기화
- `update()`: 시뮬레이션 1프레임 실행
- `updateParticle(x, y)`: 개별 입자 업데이트
- `addParticle(x, y, type)`: 입자 추가
- `getRenderBufferPtr()`: 렌더 버퍼 포인터 반환

**데이터 구조:**

- `Particle grid[]`: 현재 상태
- `Particle nextGrid[]`: 다음 프레임 계산용
- `int renderBuffer[]`: JS 전달용 (타입만)

### JavaScript (web/main.js)

**핵심 함수:**

- `loadWasm()`: Wasm 모듈 로드
- `gameLoop()`: 메인 루프 (update + render)
- `render()`: Canvas에 렌더링
- `addParticleAt(x, y)`: 마우스 입력 처리

## 🎯 확장 아이디어 (Phase 2)

### 1. 밀도 시스템

```cpp
struct Particle {
    int type;
    float temperature;
    float density;  // 추가
};
```

- 기름 (밀도 낮음) → 물 위로 뜸
- 흑요석 (밀도 높음) → 물 아래로 가라앉음

### 2. 열 전도

```cpp
// 주변 4칸의 평균 온도로 자연스럽게 열 전달
float avgTemp = (top.temp + bottom.temp + left.temp + right.temp) / 4.0f;
nextGrid[idx].temperature = (p.temperature + avgTemp) / 2.0f;
```

### 3. 화학 반응

```cpp
// 물 + 리튬 = 폭발
if (p.type == WATER && neighbor.type == LITHIUM) {
    applyExplosion(x, y, radius);
}
```

### 4. 입자 수명

```cpp
struct Particle {
    int life;  // -1 = 무한, 0 = 소멸
};
```

- 연기, 스파크 등 일시적 입자 구현

## 🐛 디버깅 팁

### Wasm 빌드 오류

```bash
# Emscripten 버전 확인
emcc --version

# 상세 빌드 로그
emcc src/simulation.cpp -o web/simulation.js -s WASM=1 -v
```

### 브라우저 콘솔 확인

- F12 → Console 탭
- Wasm 로드 오류, JavaScript 오류 확인

### 성능 측정

```javascript
// main.js의 gameLoop()에 추가
const start = performance.now();
wasmModule.update();
const end = performance.now();
console.log(`Update time: ${end - start}ms`);
```

## 📊 성능 비교 (Wasm vs JS)

순수 JS 버전을 만들어 성능 비교:

```javascript
// 순수 JS 버전 (참고용)
function updateJS() {
  for (let y = 0; y < HEIGHT; y++) {
    for (let x = 0; x < WIDTH; x++) {
      // 동일한 로직을 JS로 구현
    }
  }
}
```

**예상 결과:**

- Wasm: ~2-5ms/frame (60 FPS 가능)
- JS: ~10-30ms/frame (30-60 FPS)

## 📚 참고 자료

- [Emscripten 공식 문서](https://emscripten.org/)
- [WebAssembly MDN](https://developer.mozilla.org/en-US/docs/WebAssembly)
- [Falling Sand Game](https://en.wikipedia.org/wiki/Falling-sand_game)
- [Noita 게임](https://noitagame.com/) (고급 파우더 시뮬레이션 예시)

## 💡 최적화 팁

1. **메모리 접근 최소화**: `grid` 읽기를 한 번만
2. **캐시 친화적 순회**: 행 우선 순회 (y → x)
3. **불필요한 계산 제거**: `EMPTY` 입자는 스킵
4. **SIMD 활용** (고급): Emscripten SIMD 플래그

```bash
# SIMD 최적화 빌드 (실험적)
emcc src/simulation.cpp -o web/simulation.js -msimd128 -O3
```

## 🎓 학습 포인트

1. **WebAssembly 기초**: C++/Wasm 상호작용
2. **메모리 관리**: 공유 메모리 버퍼
3. **성능 최적화**: Wasm의 성능적 이점
4. **물리 시뮬레이션**: 셀룰러 오토마타
5. **Canvas 렌더링**: ImageData 최적화

---

**문제가 발생하면:**

1. 빌드 로그 확인
2. 브라우저 콘솔 확인
3. Emscripten 버전 확인
4. CORS 문제 → 반드시 로컬 서버 사용

**즐거운 코딩 되세요! 🚀**
