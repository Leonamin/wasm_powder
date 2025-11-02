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
  FIRE = 6,
  FROST = 7
};

// 입자 구조체
struct Particle {
  int type;          // 입자 타입 (0~7)
  float temperature; // 온도 (섭씨)

  Particle() : type(EMPTY), temperature(20.0f) {}
};

#endif // PARTICLE_H
