#include "particle.h"
#include <cstdlib>
#include <cstring>
#include <emscripten/emscripten.h>

// 그리드 크기
const int WIDTH = 400;
const int HEIGHT = 300;
const int GRID_SIZE = WIDTH * HEIGHT;

// 메모리 버퍼
Particle grid[GRID_SIZE];
Particle nextGrid[GRID_SIZE];
int renderBuffer[GRID_SIZE];

// 헬퍼 함수: 그리드 인덱스 계산
inline int getIndex(int x, int y) { return y * WIDTH + x; }

// 헬퍼 함수: 범위 체크
inline bool inBounds(int x, int y) {
  return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

// 헬퍼 함수: 빈 공간 체크
inline bool isEmpty(int x, int y) {
  if (!inBounds(x, y))
    return false;
  return grid[getIndex(x, y)].type == EMPTY;
}

// 입자 이동 함수
void moveParticle(int fromX, int fromY, int toX, int toY) {
  if (!inBounds(toX, toY))
    return;

  int fromIdx = getIndex(fromX, fromY);
  int toIdx = getIndex(toX, toY);

  nextGrid[toIdx] = grid[fromIdx];
  nextGrid[fromIdx].type = EMPTY;
  nextGrid[fromIdx].temperature = 20.0f;
}

// 입자 교환 함수
void swapParticles(int x1, int y1, int x2, int y2) {
  if (!inBounds(x1, y1) || !inBounds(x2, y2))
    return;

  int idx1 = getIndex(x1, y1);
  int idx2 = getIndex(x2, y2);

  Particle temp = nextGrid[idx1];
  nextGrid[idx1] = nextGrid[idx2];
  nextGrid[idx2] = temp;
}

// 개별 입자 업데이트
void updateParticle(int x, int y) {
  int idx = getIndex(x, y);
  Particle p = grid[idx];

  // 1. 온도에 따른 상태 전이
  switch (p.type) {
  case WATER:
    if (p.temperature >= 100.0f) {
      p.type = STEAM;
    } else if (p.temperature <= 0.0f) {
      p.type = ICE;
    }
    break;
  case ICE:
    if (p.temperature > 0.0f) {
      p.type = WATER;
    }
    break;
  case STEAM:
    if (p.temperature < 100.0f) {
      p.type = WATER;
    }
    break;
  }

  // nextGrid에 기본 상태 저장
  nextGrid[idx] = p;

  // 2. 타입별 움직임
  switch (p.type) {
  case EMPTY:
  case WALL:
  case ICE:
    // 고정: 아무것도 안 함
    break;

  case SAND:
    // 아래로 떨어짐
    if (isEmpty(x, y + 1)) {
      moveParticle(x, y, x, y + 1);
    } else if (isEmpty(x - 1, y + 1)) {
      moveParticle(x, y, x - 1, y + 1);
    } else if (isEmpty(x + 1, y + 1)) {
      moveParticle(x, y, x + 1, y + 1);
    }
    break;

  case WATER:
    // 아래 -> 대각선 아래 -> 좌우
    if (isEmpty(x, y + 1)) {
      moveParticle(x, y, x, y + 1);
    } else if (isEmpty(x - 1, y + 1)) {
      moveParticle(x, y, x - 1, y + 1);
    } else if (isEmpty(x + 1, y + 1)) {
      moveParticle(x, y, x + 1, y + 1);
    } else if (isEmpty(x - 1, y)) {
      moveParticle(x, y, x - 1, y);
    } else if (isEmpty(x + 1, y)) {
      moveParticle(x, y, x + 1, y);
    }
    break;

  case STEAM:
    // 위로 올라감 + 좌우 확산
    if (isEmpty(x, y - 1)) {
      moveParticle(x, y, x, y - 1);
    } else if (isEmpty(x - 1, y - 1)) {
      moveParticle(x, y, x - 1, y - 1);
    } else if (isEmpty(x + 1, y - 1)) {
      moveParticle(x, y, x + 1, y - 1);
    } else if (isEmpty(x - 1, y)) {
      moveParticle(x, y, x - 1, y);
    } else if (isEmpty(x + 1, y)) {
      moveParticle(x, y, x + 1, y);
    }
    break;

  case FIRE:
    // 주변 8칸의 온도를 150°C로 설정
    for (int dy = -1; dy <= 1; dy++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0)
          continue;
        int nx = x + dx;
        int ny = y + dy;
        if (inBounds(nx, ny)) {
          int nIdx = getIndex(nx, ny);
          nextGrid[nIdx].temperature = 150.0f;
        }
      }
    }
    break;

  case FROST:
    // 주변 8칸의 온도를 -20°C로 설정
    for (int dy = -1; dy <= 1; dy++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0)
          continue;
        int nx = x + dx;
        int ny = y + dy;
        if (inBounds(nx, ny)) {
          int nIdx = getIndex(nx, ny);
          nextGrid[nIdx].temperature = -20.0f;
        }
      }
    }
    break;
  }
}

// Wasm이 JS로 내보낼 함수들
extern "C" {
// 초기화
EMSCRIPTEN_KEEPALIVE
void init() {
  for (int i = 0; i < GRID_SIZE; i++) {
    grid[i].type = EMPTY;
    grid[i].temperature = 20.0f;
    nextGrid[i] = grid[i];
    renderBuffer[i] = EMPTY;
  }
}

// 시뮬레이션 1프레임 실행
EMSCRIPTEN_KEEPALIVE
void update() {
  // nextGrid를 grid 상태로 초기화
  memcpy(nextGrid, grid, sizeof(grid));

  // 모든 입자 순회 (아래에서 위로, 무작위 x 순서)
  for (int y = HEIGHT - 1; y >= 0; y--) {
    // 좌우 방향 무작위화 (더 자연스러운 움직임)
    bool leftToRight = (rand() % 2) == 0;

    if (leftToRight) {
      for (int x = 0; x < WIDTH; x++) {
        updateParticle(x, y);
      }
    } else {
      for (int x = WIDTH - 1; x >= 0; x--) {
        updateParticle(x, y);
      }
    }
  }

  // 계산 완료된 nextGrid를 grid로 복사
  memcpy(grid, nextGrid, sizeof(grid));

  // JS가 렌더링할 수 있게 render_buffer에 'type'만 복사
  for (int i = 0; i < GRID_SIZE; i++) {
    renderBuffer[i] = grid[i].type;
  }
}

// JS가 렌더 버퍼의 주소를 가져갈 함수
EMSCRIPTEN_KEEPALIVE
int *getRenderBufferPtr() { return renderBuffer; }

// JS가 마우스로 입자를 추가할 함수
EMSCRIPTEN_KEEPALIVE
void addParticle(int x, int y, int type) {
  if (!inBounds(x, y))
    return;

  int idx = getIndex(x, y);
  grid[idx].type = type;

  // 타입에 따라 초기 온도 설정
  switch (type) {
  case FIRE:
    grid[idx].temperature = 150.0f;
    break;
  case FROST:
    grid[idx].temperature = -20.0f;
    break;
  case ICE:
    grid[idx].temperature = -10.0f;
    break;
  case STEAM:
    grid[idx].temperature = 110.0f;
    break;
  default:
    grid[idx].temperature = 20.0f;
    break;
  }
}

// 그리드 크기 정보 제공
EMSCRIPTEN_KEEPALIVE
int getWidth() { return WIDTH; }

EMSCRIPTEN_KEEPALIVE
int getHeight() { return HEIGHT; }
}
