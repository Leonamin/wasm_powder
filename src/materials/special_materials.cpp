#include "special_materials.h"
#include "../core/grid.h"
#include "../core/types.h"
#include "../particle.h"
#include <cstdlib>

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
        // 주변을 가열 (임시 비활성화)
        // for (int dy = -1; dy <= 1; dy++) {
        //   for (int dx = -1; dx <= 1; dx++) {
        //     if (dx == 0 && dy == 0) continue;
        //     int nx = x + dx;
        //     int ny = y + dy;
        //     if (inBounds(nx, ny)) {
        //       int nIdx = getIndex(nx, ny);
        //       // 온도 증가 (최대 800도까지)
        //       nextGrid[nIdx].temperature += 80.0f;
        //       if (nextGrid[nIdx].temperature > 800.0f) {
        //         nextGrid[nIdx].temperature = 800.0f;
        //       }
        //       markChunkActive(nx, ny);
        //     }
        //   }
        // }
        
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
