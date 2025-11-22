#include "forces.h"
#include "../core/grid.h"
#include "../core/types.h"
#include "../material_db.h"

void updateForces() {
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
      
      // 액체 수평 가속 (퍼짐 효과 강화)
      if (p.state == STATE_LIQUID) {
        // 아래가 막혔는지 확인 (바닥이거나, 비어있지 않고 나보다 밀도가 높거나 같은 물질)
        bool blockedDown = (y >= HEIGHT - 1);
        if (!blockedDown) {
            int downIdx = getIndex(x, y + 1);
            const Particle& downP = grid[downIdx]; // 현재 상태(grid) 확인
            if (downP.type != EMPTY) {
                 const Material& downMat = getMaterial(downP.type);
                 if (downMat.density >= mat.density) {
                     blockedDown = true;
                 }
            }
        }

        if (blockedDown) {
            float flowForce = 0.5f; // 흐름 가속도 (값을 키워 반응성 향상)
            
            bool clearLeft = (x > 0 && grid[getIndex(x - 1, y)].type == EMPTY);
            bool clearRight = (x < WIDTH - 1 && grid[getIndex(x + 1, y)].type == EMPTY);
            
            if (clearLeft && !clearRight) {
                p.vx -= flowForce;
            } else if (!clearLeft && clearRight) {
                p.vx += flowForce;
            } else if (clearLeft && clearRight) {
                // 양쪽 다 비었으면 기존 속도 방향 유지하거나 랜덤
                if (std::abs(p.vx) < 0.1f) {
                    p.vx += (rand() % 2 == 0 ? flowForce : -flowForce);
                }
            }
        }
      }
      
      // 속도 제한
      if (p.vy > MAX_VELOCITY_Y) p.vy = MAX_VELOCITY_Y;
      if (p.vy < -MAX_VELOCITY_Y) p.vy = -MAX_VELOCITY_Y;
      if (p.vx > MAX_VELOCITY_X) p.vx = MAX_VELOCITY_X;
      if (p.vx < -MAX_VELOCITY_X) p.vx = -MAX_VELOCITY_X;
    }
  }
}
