#ifndef MATERIAL_DB_H
#define MATERIAL_DB_H

#include "particle.h"

// 물질 속성 구조체
struct Material {
  const char* name;              // 물질 이름
  int default_state;             // 기본 물리 상태
  float density;                 // 밀도 (kg/m³)
  float specific_heat;           // 비열 (J/(kg·K))
  float melting_point;           // 녹는점 (°C)
  float boiling_point;           // 끓는점 (°C)
  float latent_heat_fusion;      // 융해 잠열 (J/kg)
  float latent_heat_vaporization; // 기화 잠열 (J/kg)
  float viscosity;               // 점도 (Pa·s)
  int color[3];                  // RGB 색상
};

// 물질 데이터베이스
// 인덱스는 ParticleType enum과 일치해야 함
const Material g_MaterialDB[] = {
    // EMPTY (공기)
    {
        .name = "Air",
        .default_state = STATE_GAS,
        .density = 1.2f,
        .specific_heat = 1005.0f,  // 공기의 비열
        .melting_point = -999.0f,  // 상태 전이 없음
        .boiling_point = -999.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.00001f,     // 매우 낮은 점도
        .color = {0, 0, 0}         // 검정
    },
    
    // WALL (벽)
    {
        .name = "Wall",
        .default_state = STATE_SOLID,
        .density = 2500.0f,        // 콘크리트 밀도
        .specific_heat = 840.0f,   // 콘크리트 비열
        .melting_point = 1500.0f,  // 매우 높은 녹는점
        .boiling_point = 2800.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {136, 136, 136}   // 회색
    },
    
    // SAND (모래)
    {
        .name = "Sand",
        .default_state = STATE_POWDER,
        .density = 1600.0f,        // 모래 밀도
        .specific_heat = 830.0f,   // 모래 비열
        .melting_point = 1700.0f,  // 유리화 온도
        .boiling_point = 2230.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {240, 230, 140}   // 카키색
    },
    
    // WATER (물)
    {
        .name = "Water",
        .default_state = STATE_LIQUID,
        .density = 1000.0f,        // 물 밀도
        .specific_heat = 4186.0f,  // 물의 높은 비열
        .melting_point = 0.0f,     // 얼음 → 물
        .boiling_point = 100.0f,   // 물 → 증기
        .latent_heat_fusion = 334000.0f,      // 융해열
        .latent_heat_vaporization = 2260000.0f, // 기화열
        .viscosity = 0.001f,       // 물의 점도
        .color = {30, 144, 255}    // 파랑
    },
    
    // ICE (얼음)
    {
        .name = "Ice",
        .default_state = STATE_SOLID,
        .density = 917.0f,         // 얼음 밀도 (물보다 낮음)
        .specific_heat = 2050.0f,  // 얼음 비열
        .melting_point = 0.0f,     // 얼음 → 물
        .boiling_point = 100.0f,   // (물을 거쳐 증기로)
        .latent_heat_fusion = 334000.0f,
        .latent_heat_vaporization = 2260000.0f,
        .viscosity = 0.0f,
        .color = {175, 238, 238}   // 하늘색
    },
    
    // STEAM (증기)
    {
        .name = "Steam",
        .default_state = STATE_GAS,
        .density = 0.6f,           // 수증기 밀도 (공기보다 낮음)
        .specific_heat = 2080.0f,  // 수증기 비열
        .melting_point = 0.0f,
        .boiling_point = 100.0f,   // 증기 → 물
        .latent_heat_fusion = 334000.0f,
        .latent_heat_vaporization = 2260000.0f,
        .viscosity = 0.00001f,
        .color = {245, 245, 245}   // 흰색
    },
    
    // FIRE (불) - 특수 물질 (온도원)
    {
        .name = "Fire",
        .default_state = STATE_GAS,
        .density = 0.3f,           // 매우 낮은 밀도 (위로 올라감)
        .specific_heat = 1000.0f,
        .melting_point = -999.0f,  // 상태 전이 없음
        .boiling_point = -999.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {255, 69, 0}      // 주황/빨강
    }
};

// MaterialDB 크기
const int MATERIAL_COUNT = sizeof(g_MaterialDB) / sizeof(Material);

// 물질 속성 조회 헬퍼 함수
inline const Material& getMaterial(int type) {
  if (type < 0 || type >= MATERIAL_COUNT) {
    return g_MaterialDB[0]; // EMPTY 반환
  }
  return g_MaterialDB[type];
}

#endif // MATERIAL_DB_H
