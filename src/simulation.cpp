// ============================================================================
// Wasm Powder Toy - 메인 시뮬레이션 루프
// ============================================================================
#include "particle.h"
#include "material_db.h"
#include "core/grid.h"
#include "core/types.h"
#include "physics/heat_conduction.h"
#include "physics/state_change.h"
#include "physics/forces.h"
#include "physics/movement.h"
#include "materials/special_materials.h"
#include "chemistry/reaction_system.h"
#include "chemistry/reaction_registry.h"
#include <cstring>
#include <emscripten/emscripten.h>

// ============================================================================
// Wasm이 JS로 내보낼 함수들
// ============================================================================
extern "C" {

// 초기화
EMSCRIPTEN_KEEPALIVE
void init() {
  initGrid();
  
  // 화학 반응 시스템 초기화
  ReactionRegistry::getInstance().initializeAllReactions();
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
  
  // PASS 1: 화학 반응
  updateChemistry();
  
  // PASS 2: 열 전도 (임시 비활성화)
  // updateHeatConduction();
  
  // PASS 2.5: 온도 감쇠 (임시 비활성화)
  // applyCooling();
  
  // PASS 3: 상태 전이 (임시 비활성화)
  // updateStateChange();
  
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
void addParticleWrapper(int x, int y, int type) {
  addParticle(x, y, type);
}

// 그리드 크기 정보 제공
EMSCRIPTEN_KEEPALIVE
int getWidth() { return WIDTH; }

EMSCRIPTEN_KEEPALIVE
int getHeight() { return HEIGHT; }

} // extern "C"
