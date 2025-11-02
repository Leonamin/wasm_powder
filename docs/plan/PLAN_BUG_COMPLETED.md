# 해결된 버그 목록 (PLAN_BUG_COMPLETED.md)

## ✅ 완료된 버그 수정

이 파일에는 해결된 버그들이 기록됩니다.

각 항목은 다음 정보를 포함합니다:
- 버그 설명
- 해결 방법
- 수정 날짜
- 관련 파일

---

## 버그 #1: 마우스 Pressed 상태에서 물질 생성 문제

**발견 날짜:** 2025-11-02  
**해결 날짜:** 2025-11-02  
**우선순위:** 높음

### 문제 설명
- 마우스를 누르고 있는 상태에서 움직이지 않으면 물질이 계속 생성되지 않음
- 클릭 시 한 번만 생성되고 멈춤

### 해결 방법
`web/main.js`에 연속 그리기 루프 추가:

```javascript
let lastMouseX = 0;
let lastMouseY = 0;

function continuousDrawLoop() {
    if (isDrawing) {
        addParticleAt(lastMouseX, lastMouseY);
    }
    requestAnimationFrame(continuousDrawLoop);
}

// 게임 루프와 별도로 실행
continuousDrawLoop();
```

### 관련 파일
- `web/main.js` - 마우스 이벤트 핸들러 수정

---

## 버그 #2: FIRE/FROST 온도 고정 문제

**발견 날짜:** 2025-11-03  
**해결 날짜:** 2025-11-03  
**우선순위:** 높음

### 문제 설명
1. FIRE/FROST가 물질처럼 쌓여서 제거되지 않음
2. 이동한 타일의 온도를 `= 150.0f`로 덮어써서 열 전도가 작동하지 않음
3. 불이 불처럼 보이지 않고 단순히 위로 올라가기만 함

### 해결 방법

#### 1. 수명 시스템 추가
```cpp
// Particle 구조체에 life 필드 사용
if (p.life > 0) {
    p.life--;
    if (p.life == 0) {
        p.type = EMPTY; // 소멸
    }
}
```

#### 2. 온도 덮어쓰기 → 증가/감소로 변경
```cpp
// 이전: nextGrid[nIdx].temperature = 150.0f;
// 이후: nextGrid[nIdx].temperature += 30.0f;
```

#### 3. 불 확산 효과 추가
```cpp
// 뜨거운 곳에 불 확산
if (nextGrid[nIdx].type == EMPTY && nextGrid[nIdx].temperature > 80.0f) {
    nextGrid[nIdx].type = FIRE;
    nextGrid[nIdx].life = 20 + rand() % 20;
}
```

#### 4. 랜덤 움직임 추가
```cpp
int randomDir = rand() % 3 - 1; // -1, 0, 1
// 위로 + 대각선 + 좌우로 랜덤하게 이동
```

### 관련 파일
- `src/simulation.cpp` - updateLifeAndSpecialMaterials() 함수 추가
- `src/simulation.cpp` - updateMovement() 함수 수정

### 효과
- ✅ FIRE가 30-60 프레임 후 자동 소멸
- ✅ FROST가 40-80 프레임 후 자동 소멸
- ✅ 온도가 점진적으로 증가/감소하여 열 전도 정상 작동
- ✅ 불이 뜨거운 곳으로 확산되며 불처럼 보임
- ✅ 랜덤 움직임으로 자연스러운 효과

---

_현재 해결된 버그: 2개_결된 버그가 없습니다._
