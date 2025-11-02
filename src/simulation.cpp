#include "particle.h"
#include "material_db.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <emscripten/emscripten.h>

// 그리드 크기
const int WIDTH = 400;
const int HEIGHT = 300;
const int GRID_SIZE = WIDTH * HEIGHT;

// 메모리 버퍼
Particle grid[GRID_SIZE];
Particle nextGrid[GRID_SIZE];
int renderBuffer[GRID_SIZE];

// Active Chunks 시스템
const int CHUNK_SIZE = 16;
const int CHUNK_WIDTH = (WIDTH + CHUNK_SIZE - 1) / CHUNK_SIZE;
const int CHUNK_HEIGHT = (HEIGHT + CHUNK_SIZE - 1) / CHUNK_SIZE;
const int CHUNK_COUNT = CHUNK_WIDTH * CHUNK_HEIGHT;
bool activeChunks[CHUNK_COUNT];

// 헬퍼 함수: 그리드 인덱스 계산
inline int getIndex(int x, int y) { return y * WIDTH + x; }

// 헬퍼 함수: 범위 체크
inline bool inBounds(int x, int y) {
  return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

// 헬퍼 함수: 청크 인덱스 계산
inline int getChunkIndex(int x, int y) {
  int cx = x / CHUNK_SIZE;
  int cy = y / CHUNK_SIZE;
  if (cx < 0 || cx >= CHUNK_WIDTH || cy < 0 || cy >= CHUNK_HEIGHT)
    return -1;
  return cy * CHUNK_WIDTH + cx;
}

// 청크를 활성화
inline void markChunkActive(int x, int y) {
  int chunkIdx = getChunkIndex(x, y);
  if (chunkIdx >= 0 && chunkIdx < CHUNK_COUNT) {
    activeChunks[chunkIdx] = true;
  }
}

// 헬퍼 함수: 빈 공간 또는 밀도가 낮은지 체크
inline bool canMoveTo(int x, int y, float myDensity) {
  if (!inBounds(x, y))
    return false;
  
  const Particle& target = grid[getIndex(x, y)];
  if (target.type == EMPTY)
    return true;
  
  const Material& targetMat = getMaterial(target.type);
  return myDensity > targetMat.density;
}

// ============================================================================
// PASS 2: 열 전도 (Heat Conduction)
// ============================================================================
void updateHeatConduction() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = getIndex(x, y);
      const Particle& p = grid[idx];
      const Material& mat = getMaterial(p.type);
      
      // 주변 4칸의 온도 수집
      float neighborTemps[4];
      int count = 0;
      
      if (inBounds(x, y - 1)) neighborTemps[count++] = grid[getIndex(x, y - 1)].temperature;
      if (inBounds(x, y + 1)) neighborTemps[count++] = grid[getIndex(x, y + 1)].temperature;
      if (inBounds(x - 1, y)) neighborTemps[count++] = grid[getIndex(x - 1, y)].temperature;
      if (inBounds(x + 1, y)) neighborTemps[count++] = grid[getIndex(x + 1, y)].temperature;
      
      if (count == 0) continue;
      
      // 평균 온도 계산
      float avgTemp = 0.0f;
      for (int i = 0; i < count; i++) {
        avgTemp += neighborTemps[i];
      }
      avgTemp /= count;
      
      // 비열을 고려한 열 전도율 계산
      // 비열이 높을수록 온도 변화가 느림
      float conductionRate = 0.05f / (mat.specific_heat / 1000.0f);
      
      // 새 온도 계산
      float newTemp = p.temperature + (avgTemp - p.temperature) * conductionRate;
      nextGrid[idx].temperature = newTemp;
      
      // 온도가 변하면 청크 활성화
      if (fabs(newTemp - p.temperature) > 0.1f) {
        markChunkActive(x, y);
      }
    }
  }
}

// ============================================================================
// PASS 3: 상태 전이 (State Change)
// ============================================================================
void updateStateChange() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = getIndex(x, y);
      Particle& p = nextGrid[idx];
      
      if (p.type == EMPTY || p.type == WALL) continue;
      
      const Material& mat = getMaterial(p.type);
      
      // 특수 물질 (FIRE, FROST)은 상태 전이 없음
      if (p.type == FIRE ) continue;
      
      // 녹는점 체크 (고체 → 액체)
      if (p.temperature > mat.melting_point && p.type == ICE) {
        // 간단한 버전: 잠열 없이 즉시 변환
        p.type = WATER;
        p.state = STATE_LIQUID;
        markChunkActive(x, y);
      }
      // 끓는점 체크 (액체 → 기체)
      else if (p.temperature >= mat.boiling_point && p.type == WATER) {
        p.type = STEAM;
        p.state = STATE_GAS;
        markChunkActive(x, y);
      }
      // 응고점 체크 (액체 → 고체)
      else if (p.temperature <= mat.melting_point && p.type == WATER) {
        p.type = ICE;
        p.state = STATE_SOLID;
        markChunkActive(x, y);
      }
      // 응축점 체크 (기체 → 액체)
      else if (p.temperature < mat.boiling_point && p.type == STEAM) {
        p.type = WATER;
        p.state = STATE_LIQUID;
        markChunkActive(x, y);
      }
    }
  }
}

// ============================================================================
// PASS 4: 힘 계산 (Forces)
// ============================================================================
void updateForces() {
  const float GRAVITY = 0.3f;
  
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = getIndex(x, y);
      Particle& p = nextGrid[idx];
      
      if (p.type == EMPTY || p.type == WALL) continue;
      if (p.state == STATE_SOLID) continue;
      
      const Material& mat = getMaterial(p.type);
      
      // 중력 적용 (밀도에 비례)
      // 밀도가 공기(1.2)보다 높으면 아래로, 낮으면 위로
      float densityRatio = (mat.density - 1.2f) / 1000.0f;
      p.vy += GRAVITY * densityRatio;
      
      // 속도 제한
      if (p.vy > 5.0f) p.vy = 5.0f;
      if (p.vy < -5.0f) p.vy = -5.0f;
      if (p.vx > 3.0f) p.vx = 3.0f;
      if (p.vx < -3.0f) p.vx = -3.0f;
    }
  }
}

// ============================================================================
// PASS 4.5: 수명 감소 및 특수 물질 처리
// ============================================================================
void updateLifeAndSpecialMaterials() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = getIndex(x, y);
      Particle& p = nextGrid[idx];
      
      if (p.type == EMPTY || p.type == WALL) continue;
      
      // 수명 감소
      if (p.life > 0) {
        p.life--;
        if (p.life == 0) {
          // 수명 다하면 소멸
          p.type = EMPTY;
          p.state = STATE_GAS;
          markChunkActive(x, y);
          continue;
        }
      }
      
      // FIRE: 주변을 가열하고 위로 올라가며 소멸
      if (p.type == FIRE) {
        // 주변을 가열 (덮어쓰기 대신 증가)
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (inBounds(nx, ny)) {
              int nIdx = getIndex(nx, ny);
              // 온도 증가 (최대 200도까지)
              nextGrid[nIdx].temperature += 30.0f;
              if (nextGrid[nIdx].temperature > 200.0f) {
                nextGrid[nIdx].temperature = 200.0f;
              }
              markChunkActive(nx, ny);
            }
          }
        }
        
        // 랜덤하게 확산 (부모보다 life 감소)
        if (rand() % 3 == 0 && p.life > 10) { // life가 10 이상일 때만 확산
          int dir = rand() % 4;
          int nx = x + (dir == 0 ? -1 : dir == 1 ? 1 : 0);
          int ny = y + (dir == 2 ? -1 : dir == 3 ? 1 : 0);
          
          if (inBounds(nx, ny)) {
            int nIdx = getIndex(nx, ny);
            if (nextGrid[nIdx].type == EMPTY && nextGrid[nIdx].temperature > 80.0f) {
              // 뜨거운 곳에 불 확산 (부모보다 life 5-10 감소)
              int newLife = p.life - 5 - rand() % 6;
              if (newLife > 0) {
                nextGrid[nIdx].type = FIRE;
                nextGrid[nIdx].state = STATE_GAS;
                nextGrid[nIdx].life = newLife;
                markChunkActive(nx, ny);
              }
            }
          }
        }
      }
      
    }
  }
}

// ============================================================================
// PASS 5: 이동 및 교환 (Movement)
// ============================================================================
void updateMovement() {
  // 아래에서 위로, 랜덤 좌우 순서로 순회
  for (int y = HEIGHT - 1; y >= 0; y--) {
    bool leftToRight = (rand() % 2) == 0;
    
    int startX = leftToRight ? 0 : WIDTH - 1;
    int endX = leftToRight ? WIDTH : -1;
    int stepX = leftToRight ? 1 : -1;
    
    for (int x = startX; x != endX; x += stepX) {
      int idx = getIndex(x, y);
      Particle& p = nextGrid[idx];
      
      if (p.type == EMPTY || p.type == WALL) continue;
      if (p.state == STATE_SOLID) continue;
      if (p.updated_this_frame) continue;
      
      const Material& mat = getMaterial(p.type);
      
      // FIRE: 위로 올라감 + 랜덤 움직임
      if (p.type == FIRE) {
        // 랜덤 방향 추가
        int randomDir = rand() % 3 - 1; // -1, 0, 1
        
        if (canMoveTo(x, y - 1, mat.density)) {
          int toIdx = getIndex(x, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x, y - 1);
        } else if (randomDir != 0 && canMoveTo(x + randomDir, y - 1, mat.density)) {
          int toIdx = getIndex(x + randomDir, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x + randomDir, y - 1);
        } else if (canMoveTo(x + randomDir, y, mat.density)) {
          int toIdx = getIndex(x + randomDir, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x + randomDir, y);
        }
        continue;
      }
      
      
      // 일반 물질 이동
      bool moved = false;
      
      // 속도 기반 목표 위치 계산
      int targetY = y + (int)p.vy;
      int targetX = x + (int)p.vx;
      
      // POWDER: 아래로 떨어짐
      if (p.state == STATE_POWDER) {
        if (canMoveTo(x, y + 1, mat.density)) {
          int toIdx = getIndex(x, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x, y + 1);
        } else if (canMoveTo(x - 1, y + 1, mat.density)) {
          int toIdx = getIndex(x - 1, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x - 1, y + 1);
        } else if (canMoveTo(x + 1, y + 1, mat.density)) {
          int toIdx = getIndex(x + 1, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x + 1, y + 1);
        }
      }
      // LIQUID: 아래 + 좌우로 퍼짐
      else if (p.state == STATE_LIQUID) {
        if (canMoveTo(x, y + 1, mat.density)) {
          int toIdx = getIndex(x, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x, y + 1);
        } else if (canMoveTo(x - 1, y + 1, mat.density)) {
          int toIdx = getIndex(x - 1, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x - 1, y + 1);
        } else if (canMoveTo(x + 1, y + 1, mat.density)) {
          int toIdx = getIndex(x + 1, y + 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x + 1, y + 1);
        } else if (canMoveTo(x - 1, y, mat.density)) {
          int toIdx = getIndex(x - 1, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x - 1, y);
        } else if (canMoveTo(x + 1, y, mat.density)) {
          int toIdx = getIndex(x + 1, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x + 1, y);
        }
      }
      // GAS: 위로 올라감 + 확산
      else if (p.state == STATE_GAS) {
        if (canMoveTo(x, y - 1, mat.density)) {
          int toIdx = getIndex(x, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x, y - 1);
        } else if (canMoveTo(x - 1, y - 1, mat.density)) {
          int toIdx = getIndex(x - 1, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x - 1, y - 1);
        } else if (canMoveTo(x + 1, y - 1, mat.density)) {
          int toIdx = getIndex(x + 1, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x + 1, y - 1);
        } else if (canMoveTo(x - 1, y, mat.density)) {
          int toIdx = getIndex(x - 1, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x - 1, y);
        } else if (canMoveTo(x + 1, y, mat.density)) {
          int toIdx = getIndex(x + 1, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          moved = true;
          markChunkActive(x, y);
          markChunkActive(x + 1, y);
        }
      }
      
      // 속도 감쇠
      if (!moved) {
        nextGrid[idx].vx *= 0.8f;
        nextGrid[idx].vy *= 0.8f;
      }
    }
  }
}

// ============================================================================
// 렌더 버퍼 업데이트
// ============================================================================
void updateRenderBuffer() {
  for (int i = 0; i < GRID_SIZE; i++) {
    renderBuffer[i] = grid[i].type;
  }
}

// ============================================================================
// Wasm이 JS로 내보낼 함수들
// ============================================================================
extern "C" {

// 초기화
EMSCRIPTEN_KEEPALIVE
void init() {
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

// 시뮬레이션 1프레임 실행
EMSCRIPTEN_KEEPALIVE
void update() {
  // PASS 0: 준비
  memcpy(nextGrid, grid, sizeof(grid));
  
  // updated_this_frame 플래그 초기화
  for (int i = 0; i < GRID_SIZE; i++) {
    nextGrid[i].updated_this_frame = false;
  }
  
  // PASS 1: 화학 반응 (나중에 구현)
  // updateChemistry();
  
  // PASS 2: 열 전도
  updateHeatConduction();
  
  // PASS 3: 상태 전이
  updateStateChange();
  
  // PASS 4: 힘 계산
  updateForces();
  
  // PASS 4.5: 수명 및 특수 물질
  updateLifeAndSpecialMaterials();
  
  // PASS 5: 이동
  updateMovement();
  
  // FINAL: 그리드 교체
  memcpy(grid, nextGrid, sizeof(grid));
  
  // 렌더 버퍼 업데이트
  updateRenderBuffer();
}

// JS가 렌더 버퍼의 주소를 가져갈 함수
EMSCRIPTEN_KEEPALIVE
int* getRenderBufferPtr() { 
  return renderBuffer; 
}

// JS가 Particle 배열의 주소를 가져갈 함수 (온도 시각화용)
EMSCRIPTEN_KEEPALIVE
Particle* getParticleArrayPtr() {
  return grid;
}

// Particle 구조체 크기 반환
EMSCRIPTEN_KEEPALIVE
int getParticleSize() {
  return sizeof(Particle);
}

// JS가 마우스로 입자를 추가할 함수
EMSCRIPTEN_KEEPALIVE
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

// 그리드 크기 정보 제공
EMSCRIPTEN_KEEPALIVE
int getWidth() { return WIDTH; }

EMSCRIPTEN_KEEPALIVE
int getHeight() { return HEIGHT; }

} // extern "C"
