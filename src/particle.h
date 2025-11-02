#ifndef PARTICLE_H
#define PARTICLE_H

// 입자 타입 상수
enum ParticleType {
  EMPTY = 0,
  WALL = 1,
  SAND = 2,
  WATER = 3,
  ICE = 4,
  STEAM = 5,
  FIRE = 6
};

// 물리 상태 (material_db.h와 동일)
enum PhysicalState {
  STATE_SOLID = 0,   // 고체 (움직이지 않음)
  STATE_POWDER = 1,  // 가루 (아래로 떨어짐)
  STATE_LIQUID = 2,  // 액체 (아래로 떨어지고 퍼짐)
  STATE_GAS = 3      // 기체 (위로 올라감)
};

// 입자 구조체 (셀 상태)
struct Particle {
  // === 1. 물질 그리드 ===
  int type;          // 물질 ID (0=EMPTY, 1=WALL, 2=SAND...)

  // === 2. 환경 그리드 (배경 속성) ===
  float temperature; // 현재 셀의 온도 (EMPTY라도 값을 가짐)
  // float pressure; // [Phase 3] 현재 셀의 기압

  // === 3. 물리 상태 (물질이 있을 경우) ===
  int state;         // 물리 상태 (SOLID, POWDER, LIQUID, GAS)
  float vx, vy;      // 속도 (픽셀/프레임)
  float latent_heat_storage; // 축적된 잠열 (상태 전이용)

  // === 4. 기타 상태 ===
  int life;          // 수명 (-1 = 무한, 0 = 소멸)
  bool updated_this_frame; // 이동 최적화용 플래그

  // 생성자
  Particle() 
    : type(EMPTY), 
      temperature(20.0f), 
      state(STATE_SOLID),
      vx(0.0f), 
      vy(0.0f), 
      latent_heat_storage(0.0f),
      life(-1), 
      updated_this_frame(false) {}
};

#endif // PARTICLE_H
