#ifndef GRID_H
#define GRID_H

#include "../particle.h"
#include "types.h"

// 그리드 데이터
extern Particle grid[GRID_SIZE];
extern Particle nextGrid[GRID_SIZE];
extern int renderBuffer[GRID_SIZE];

// 청크 시스템 (현재 항상 활성화)
extern bool activeChunks[CHUNK_COUNT];

// 헬퍼 함수: 그리드 인덱스 계산
inline int getIndex(int x, int y) { 
  return y * WIDTH + x; 
}

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

// 그리드 초기화
void initGrid();

// 렌더 버퍼 업데이트
void updateRenderBuffer();

// 입자 추가
void addParticle(int x, int y, int type);

#endif // GRID_H
