#include "grid.h"
#include "../material_db.h"
#include <cstring>
#include <cstdlib>

// 그리드 데이터 정의
Particle grid[GRID_SIZE];
Particle nextGrid[GRID_SIZE];
int renderBuffer[GRID_SIZE];
bool activeChunks[CHUNK_COUNT];

// 그리드 초기화
void initGrid() {
  for (int i = 0; i < GRID_SIZE; i++) {
    grid[i] = Particle(); // 기본 생성자 사용
    nextGrid[i] = grid[i];
    renderBuffer[i] = EMPTY;
  }
  
  // 모든 청크 활성화
  for (int i = 0; i < CHUNK_COUNT; i++) {
    activeChunks[i] = true;
  }
}

// 렌더 버퍼 업데이트
void updateRenderBuffer() {
  for (int i = 0; i < GRID_SIZE; i++) {
    renderBuffer[i] = grid[i].type;
  }
}

// 입자 추가
void addParticle(int x, int y, int type) {
  if (!inBounds(x, y))
    return;

  int idx = getIndex(x, y);
  grid[idx].type = type;
  
  const Material& mat = getMaterial(type);
  grid[idx].state = mat.default_state;
  
  // 타입에 따라 초기 온도 및 수명 설정
  switch (type) {
  case FIRE:
    grid[idx].temperature = 150.0f;
    grid[idx].life = 30 + rand() % 30; // 30-60 프레임 (0.5-1초)
    break;
  case ICE:
    grid[idx].temperature = -10.0f;
    grid[idx].life = -1; // 무한
    break;
  case STEAM:
    grid[idx].temperature = 110.0f;
    grid[idx].life = -1; // 무한
    break;
  default:
    grid[idx].temperature = 20.0f;
    grid[idx].life = -1; // 무한
    break;
  }
  
  // 속도 초기화
  grid[idx].vx = 0.0f;
  grid[idx].vy = 0.0f;
  grid[idx].latent_heat_storage = 0.0f;
  
  // 청크 활성화
  markChunkActive(x, y);
}
