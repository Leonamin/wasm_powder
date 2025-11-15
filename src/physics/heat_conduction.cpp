#include "heat_conduction.h"
#include "../core/grid.h"
#include "../core/types.h"
#include "../material_db.h"
#include <cmath>

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
      float conductionRate = HEAT_CONDUCTION_BASE / (mat.specific_heat / 1000.0f);
      
      // 새 온도 계산
      float newTemp = p.temperature + (avgTemp - p.temperature) * conductionRate;
      nextGrid[idx].temperature = newTemp;
      
      // 온도가 변하면 청크 활성화
      if (fabs(newTemp - p.temperature) > HEAT_CHANGE_THRESHOLD) {
        markChunkActive(x, y);
      }
    }
  }
}
