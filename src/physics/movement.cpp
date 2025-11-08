#include "movement.h"
#include "../core/grid.h"
#include "../core/types.h"
#include "../material_db.h"
#include <cstdlib>

// 헬퍼 함수: 빈 공간 또는 밀도가 낮은지 체크
static bool canMoveTo(int x, int y, float myDensity) {
  if (!inBounds(x, y))
    return false;
  
  const Particle& target = grid[getIndex(x, y)];
  if (target.type == EMPTY)
    return true;
  
  const Material& targetMat = getMaterial(target.type);
  return myDensity > targetMat.density;
}

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
        nextGrid[idx].vx *= VELOCITY_DAMPING;
        nextGrid[idx].vy *= VELOCITY_DAMPING;
      }
    }
  }
}
