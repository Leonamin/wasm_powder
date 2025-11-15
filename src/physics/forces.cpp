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
      
      // 속도 제한
      if (p.vy > MAX_VELOCITY_Y) p.vy = MAX_VELOCITY_Y;
      if (p.vy < -MAX_VELOCITY_Y) p.vy = -MAX_VELOCITY_Y;
      if (p.vx > MAX_VELOCITY_X) p.vx = MAX_VELOCITY_X;
      if (p.vx < -MAX_VELOCITY_X) p.vx = -MAX_VELOCITY_X;
    }
  }
}
