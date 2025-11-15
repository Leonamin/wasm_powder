#include "state_change.h"
#include "../core/grid.h"
#include "../core/types.h"
#include "../material_db.h"

void updateStateChange() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = getIndex(x, y);
      Particle& p = nextGrid[idx];
      
      if (p.type == EMPTY || p.type == WALL) continue;
      
      const Material& mat = getMaterial(p.type);
      
      // 특수 물질 (FIRE)은 상태 전이 없음
      if (p.type == FIRE) continue;
      
      // 녹는점 체크 (고체 → 액체)
      if (p.temperature > mat.melting_point && p.type == ICE) {
        // 얼음이 물로 변환
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
