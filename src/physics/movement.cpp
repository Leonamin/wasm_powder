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
      if (p.updated_this_frame) continue;
      
      const Material& mat = getMaterial(p.type);
      
      // 일반 고체는 움직이지 않음
      if (p.state == STATE_SOLID) continue;
      
      // FIRE: 위로 올라감 + 랜덤 움직임
      if (p.type == FIRE) {
        bool fireMoved = false;
        // 랜덤 방향 추가
        int randomDir = rand() % 3 - 1; // -1, 0, 1
        
        // 1. 위로 이동 시도 (직진 또는 대각선)
        if (canMoveTo(x, y - 1, mat.density)) {
          int toIdx = getIndex(x, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x, y - 1);
          fireMoved = true;
        } else if (randomDir != 0 && canMoveTo(x + randomDir, y - 1, mat.density)) {
          int toIdx = getIndex(x + randomDir, y - 1);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x + randomDir, y - 1);
          fireMoved = true;
        } else if (canMoveTo(x + randomDir, y, mat.density)) { // 2. 랜덤 좌우 이동 시도 (1칸)
          int toIdx = getIndex(x + randomDir, y);
          Particle temp = nextGrid[idx];
          nextGrid[idx] = nextGrid[toIdx];
          nextGrid[toIdx] = temp;
          nextGrid[toIdx].updated_this_frame = true;
          markChunkActive(x, y);
          markChunkActive(x + randomDir, y);
          fireMoved = true;
        }
        
        // 3. 이동 실패 시 수평 확산 (Slide) 시도 - 불이 갇히는 것 방지
        if (!fireMoved) {
            int horizDir = (rand() % 2) * 2 - 1; // -1 또는 1
            int fireDispersion = 3; // 불은 기체보다 덜 퍼지지만 어느 정도 미끄러져야 함
            
            for (int dist = 1; dist <= fireDispersion; dist++) {
              if (canMoveTo(x + horizDir * dist, y, mat.density)) {
                int toIdx = getIndex(x + horizDir * dist, y);
                Particle temp = nextGrid[idx];
                nextGrid[idx] = nextGrid[toIdx];
                nextGrid[toIdx] = temp;
                nextGrid[toIdx].updated_this_frame = true;
                markChunkActive(x, y);
                markChunkActive(x + horizDir * dist, y);
                break;
              }
            }
            // 반대 방향 시도 생략 (성능 고려, 다음 프레임에 시도)
        }
        continue;
      }
      
      
      // 일반 물질 이동
      bool moved = false;
      
      // 속도 기반 목표 위치 계산
      int targetY = y + (int)p.vy;
      int targetX = x + (int)p.vx;
      
      // POWDER: 아래로 떨어짐 + 랜덤 좌우 움직임
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
        } else {
          // 대각선 방향 랜덤 선택
          int dir = (rand() % 2) * 2 - 1; // -1 또는 1
          if (canMoveTo(x + dir, y + 1, mat.density)) {
            int toIdx = getIndex(x + dir, y + 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x + dir, y + 1);
          } else if (canMoveTo(x - dir, y + 1, mat.density)) {
            int toIdx = getIndex(x - dir, y + 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x - dir, y + 1);
          }
        }
      }
      // LIQUID: 아래 + 좌우로 퍼짐 (향상된 확산)
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
        } else {
          // 이동 방향 결정 (vx가 있으면 관성 따름, 없으면 랜덤)
          int preferredDir = 0;
          if (std::abs(p.vx) > 0.1f) {
            preferredDir = (p.vx > 0) ? 1 : -1;
          } else {
            preferredDir = (rand() % 2) * 2 - 1; // -1 또는 1
          }

          // 대각선 이동 시도 (선호 방향 우선)
          if (canMoveTo(x + preferredDir, y + 1, mat.density)) {
            int toIdx = getIndex(x + preferredDir, y + 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x + preferredDir, y + 1);
          } else if (canMoveTo(x - preferredDir, y + 1, mat.density)) { // 반대쪽 대각선
            int toIdx = getIndex(x - preferredDir, y + 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x - preferredDir, y + 1);
          } else {
            // 수평 확산
            int horizDir = preferredDir;
            int dispersionRate = 10; 
            
            for (int dist = 1; dist <= dispersionRate; dist++) {
              if (canMoveTo(x + horizDir * dist, y, mat.density)) {
                int toIdx = getIndex(x + horizDir * dist, y);
                Particle temp = nextGrid[idx];
                nextGrid[idx] = nextGrid[toIdx];
                nextGrid[toIdx] = temp;
                nextGrid[toIdx].updated_this_frame = true;
                moved = true;
                markChunkActive(x, y);
                markChunkActive(x + horizDir * dist, y);
                break;
              }
            }
            
            // 반대 방향도 시도 (vx가 있어도 막히면 반대로 갈 수 있어야 함)
            if (!moved) {
              for (int dist = 1; dist <= dispersionRate; dist++) {
                if (canMoveTo(x - horizDir * dist, y, mat.density)) {
                  int toIdx = getIndex(x - horizDir * dist, y);
                  Particle temp = nextGrid[idx];
                  nextGrid[idx] = nextGrid[toIdx];
                  nextGrid[toIdx] = temp;
                  nextGrid[toIdx].updated_this_frame = true;
                  moved = true;
                  markChunkActive(x, y);
                  markChunkActive(x - horizDir * dist, y);
                  break;
                }
              }
            }
          }
        }
      }
      // GAS: 위로 올라감 + 랜덤 확산
      else if (p.state == STATE_GAS) {
        // 랜덤 움직임 방향 선택
        int randomChoice = rand() % 10;
        
        // 70% 확률로 위로 이동
        if (randomChoice < 7) {
          int diagDir = (rand() % 2) * 2 - 1; // -1 또는 1
          
          if (canMoveTo(x, y - 1, mat.density)) {
            int toIdx = getIndex(x, y - 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x, y - 1);
          } else if (canMoveTo(x + diagDir, y - 1, mat.density)) {
            int toIdx = getIndex(x + diagDir, y - 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x + diagDir, y - 1);
          } else if (canMoveTo(x - diagDir, y - 1, mat.density)) {
            int toIdx = getIndex(x - diagDir, y - 1);
            Particle temp = nextGrid[idx];
            nextGrid[idx] = nextGrid[toIdx];
            nextGrid[toIdx] = temp;
            nextGrid[toIdx].updated_this_frame = true;
            moved = true;
            markChunkActive(x, y);
            markChunkActive(x - diagDir, y - 1);
          }
        }
        
        // 이동하지 못했으면 수평 확산
        if (!moved) {
          int horizDir = (rand() % 2) * 2 - 1; // -1 또는 1
          int dispersionRate = 5; // 기체 확산 거리 증가 (2 -> 5)
          
          for (int dist = 1; dist <= dispersionRate; dist++) {
            if (canMoveTo(x + horizDir * dist, y, mat.density)) {
              int toIdx = getIndex(x + horizDir * dist, y);
              Particle temp = nextGrid[idx];
              nextGrid[idx] = nextGrid[toIdx];
              nextGrid[toIdx] = temp;
              nextGrid[toIdx].updated_this_frame = true;
              moved = true;
              markChunkActive(x, y);
              markChunkActive(x + horizDir * dist, y);
              break;
            }
          }
          
          if (!moved) {
            for (int dist = 1; dist <= dispersionRate; dist++) {
              if (canMoveTo(x - horizDir * dist, y, mat.density)) {
                int toIdx = getIndex(x - horizDir * dist, y);
                Particle temp = nextGrid[idx];
                nextGrid[idx] = nextGrid[toIdx];
                nextGrid[toIdx] = temp;
                nextGrid[toIdx].updated_this_frame = true;
                moved = true;
                markChunkActive(x, y);
                markChunkActive(x - horizDir * dist, y);
                break;
              }
            }
          }
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
