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
    },
    
    // OXYGEN (산소)
    {
        .name = "Oxygen",
        .default_state = STATE_GAS,
        .density = 1.4f,           // 공기보다 약간 무거움
        .specific_heat = 920.0f,
        .melting_point = -218.0f,
        .boiling_point = -183.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.00002f,
        .color = {200, 220, 255}   // 연한 파랑
    },
    
    // HYDROGEN (수소)
    {
        .name = "Hydrogen",
        .default_state = STATE_GAS,
        .density = 0.09f,          // 매우 가벼움 (빠르게 위로)
        .specific_heat = 14300.0f, // 매우 높은 비열
        .melting_point = -259.0f,
        .boiling_point = -253.0f,
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.00001f,
        .color = {255, 200, 200}   // 연한 빨강
    },
    
    // STEAM_OIL (유증기)
    {
        .name = "Oil Steam",
        .default_state = STATE_GAS,
        .density = 2.5f,           // 수증기보다 무거움
        .specific_heat = 2000.0f,
        .melting_point = -999.0f,
        .boiling_point = 300.0f,   // 300°C 이하로 냉각되면 기름으로
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 500000.0f,
        .viscosity = 0.00003f,
        .color = {80, 70, 50}      // 어두운 갈색 증기
    },
    
    // WOOD (나무)
    {
        .name = "Wood",
        .default_state = STATE_SOLID,
        .density = 600.0f,         // 물보다 가벼움
        .specific_heat = 1700.0f,
        .melting_point = -999.0f,  // 녹지 않음
        .boiling_point = -999.0f,  // 발화점은 화학반응으로 처리
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.0f,
        .color = {139, 69, 19}     // 갈색
    },
    
    // IRON (철)
    {
        .name = "Iron",
        .default_state = STATE_SOLID,
        .density = 7874.0f,        // 매우 무거움
        .specific_heat = 449.0f,
        .melting_point = 1538.0f,  // 높은 녹는점
        .boiling_point = 2862.0f,
        .latent_heat_fusion = 247000.0f,
        .latent_heat_vaporization = 6090000.0f,
        .viscosity = 0.0f,
        .color = {192, 192, 192}   // 은색
    },
    
    // LITHIUM (리튬)
    {
        .name = "Lithium",
        .default_state = STATE_POWDER,
        .density = 534.0f,         // 가벼운 금속
        .specific_heat = 3582.0f,  // 높은 비열
        .melting_point = 180.5f,
        .boiling_point = 1342.0f,
        .latent_heat_fusion = 432000.0f,
        .latent_heat_vaporization = 20900000.0f,
        .viscosity = 0.0f,
        .color = {220, 220, 220}   // 밝은 회색
    },
    
    // SODIUM (나트륨)
    {
        .name = "Sodium",
        .default_state = STATE_POWDER,
        .density = 971.0f,         // 물보다 약간 가벼움
        .specific_heat = 1230.0f,
        .melting_point = 97.7f,
        .boiling_point = 883.0f,
        .latent_heat_fusion = 113000.0f,
        .latent_heat_vaporization = 4210000.0f,
        .viscosity = 0.0f,
        .color = {200, 200, 210}   // 은백색
    },
    
    // OIL (기름)
    {
        .name = "Oil",
        .default_state = STATE_LIQUID,
        .density = 900.0f,         // 물보다 가벼움 (위로 뜸)
        .specific_heat = 2000.0f,
        .melting_point = -40.0f,
        .boiling_point = 300.0f,   // 300°C에서 유증기로
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 500000.0f,
        .viscosity = 0.05f,        // 물보다 점성 높음
        .color = {100, 80, 40}     // 어두운 갈색
    },
    
    // CO2 (이산화탄소)
    {
        .name = "CO2",
        .default_state = STATE_GAS,
        .density = 1.98f,          // 공기보다 무거움 (아래로 가라앉음)
        .specific_heat = 840.0f,
        .melting_point = -78.5f,   // 드라이아이스
        .boiling_point = -78.5f,   // 승화
        .latent_heat_fusion = 0.0f,
        .latent_heat_vaporization = 0.0f,
        .viscosity = 0.00001f,
        .color = {180, 180, 180}   // 회색 (무거운 기체)
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
