# 버그 목록 (PLAN_BUG.md)

## 🐛 현재 발견된 버그

_현재 발견된 버그가 없습니다._

---

## 📝 해결된 버그 (PLAN_BUG_COMPLETED.md로 이동됨)

### 1. 마우스 Pressed 상태에서 물질 생성 문제 ✅

**문제 설명:**
- 현재 구현에서는 마우스 클릭(mousedown) 또는 마우스 이동(mousemove) 시에만 물질이 생성됨
- 마우스를 누르고 있는 상태(pressed)에서 움직이지 않으면 물질이 계속 생성되지 않음
- 사용자가 한 지점에 마우스를 누르고 있을 때 연속적으로 물질이 생성되어야 하는데, 현재는 클릭 시 한 번만 생성됨

**현재 코드 위치:**
- `web/main.js` 82-99번째 줄 (마우스 이벤트 핸들러)

**현재 동작:**
```javascript
canvas.addEventListener('mousedown', (e) => {
    isDrawing = true;
    addParticleAtMouse(e);  // 클릭 시 한 번만 호출
});

canvas.addEventListener('mousemove', (e) => {
    if (isDrawing) {
        addParticleAtMouse(e);  // 움직일 때만 호출
    }
});
```

**기대 동작:**
- 마우스를 누르고 있는 동안(pressed) 지속적으로 물질이 생성되어야 함
- 마우스를 움직이지 않아도 같은 위치에 계속 물질이 추가되어야 함

**해결 방안:**
1. `requestAnimationFrame` 또는 `setInterval`을 사용하여 마우스가 눌려있는 동안 지속적으로 입자 생성
2. `mousedown` 시 타이머 시작, `mouseup` 시 타이머 중지
3. 마우스 위치를 추적하여 현재 위치에 계속 입자 추가

**해결 날짜:** 2025-11-02

### 2. FIRE/FROST 온도 고정 문제 ✅

**문제 설명:**
- FIRE/FROST가 물질처럼 쌓여서 제거되지 않음
- 이동한 타일의 온도를 덮어써서 열 전도가 작동하지 않음
- 불이 불처럼 보이지 않고 단순히 위로 올라가기만 함

**해결 날짜:** 2025-11-03

---

## 📝 참고사항

- 해결된 버그는 `PLAN_BUG_COMPLETED.md`로 이동
- 각 버그는 해결 시 해결 방법과 커밋 정보를 함께 기록
