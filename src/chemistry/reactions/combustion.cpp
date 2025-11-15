#include "combustion.h"
#include "../reaction_system.h"

// 나무 + 불 → 불 + CO2 (불완전 연소)
ReactionResult react_wood_fire(const Particle& wood, const Particle& fire, int wx, int wy, int fx, int fy) {
    ReactionResult result;
    
    // 확률적 반응 (50% 확률 - 불이 닿으면 잘 탐)
    if (randomFloat() > 0.5f) {
        return result;
    }
    
    // 반응 발생 - 불완전 연소
    result.occurred = true;
    result.new_type_center = FIRE;      // 나무 → 불로 변환
    result.new_type_neighbor = CO2;     // 불 → CO2 (불완전 연소 생성물)
    result.heat_released = 15000.0f;    // 발열 반응 (15kJ)
    result.life_center = 30 + rand() % 30;  // 불 수명: 30~60 프레임
    result.life_neighbor = -1;          // CO2는 무한
    
    return result;
}

// 기름 + 불 → 불 + CO2 (불완전 연소, 더 강력)
ReactionResult react_oil_fire(const Particle& oil, const Particle& fire, int oilx, int oily, int fx, int fy) {
    ReactionResult result;
    
    // 확률적 반응 (70% 확률 - 나무보다 더 잘 탐)
    if (randomFloat() > 0.7f) {
        return result;
    }
    
    // 반응 발생 - 불완전 연소
    result.occurred = true;
    result.new_type_center = FIRE;      // 기름 → 불
    result.new_type_neighbor = CO2;     // 불 → CO2
    result.heat_released = 30000.0f;    // 더 강한 발열 (30kJ)
    result.life_center = 40 + rand() % 40;  // 불 수명: 40~80 프레임 (기름이 더 오래 탐)
    result.life_neighbor = -1;          // CO2는 무한
    
    return result;
}

// 수소 + 불 → 불 + 증기 (폭발적 연소)
ReactionResult react_hydrogen_fire(const Particle& hydrogen, const Particle& fire, int hx, int hy, int fx, int fy) {
    ReactionResult result;
    
    // 확률적 반응 (80% 확률 - 매우 반응성 높음)
    if (randomFloat() > 0.8f) {
        return result;
    }
    
    // 반응 발생 - 폭발적 반응!
    result.occurred = true;
    result.new_type_center = FIRE;      // 수소 → 불
    result.new_type_neighbor = STEAM;   // 불 → 수증기 (완전 연소)
    result.heat_released = 50000.0f;    // 매우 강한 발열 (50kJ)
    result.explosion_radius = 5;        // 폭발 반경 5칸
    result.explosion_force = 3.0f;      // 강한 폭발력
    result.life_center = 20 + rand() % 20;  // 불 수명: 20~40 프레임 (빠르게 소진)
    result.life_neighbor = -1;          // 증기는 무한
    
    return result;
}

// 연소 반응 등록
void registerCombustionReactions(ReactionRegistry& registry) {
    // 나무 + 불 → 불 + CO2
    registry.registerReaction({
        .reactant_a = WOOD,
        .reactant_b = FIRE,
        .handler = react_wood_fire,
        .probability = 0.5f,
        .min_temperature = -999.0f,  // 온도 조건 없음
        .name = "Wood Combustion"
    });
    
    // 불 + 나무 (순서 반대)
    registry.registerReaction({
        .reactant_a = FIRE,
        .reactant_b = WOOD,
        .handler = [](const Particle& f, const Particle& w, int fx, int fy, int wx, int wy) {
            return react_wood_fire(w, f, wx, wy, fx, fy);
        },
        .probability = 0.5f,
        .min_temperature = -999.0f,
        .name = "Wood Combustion (rev)"
    });
    
    // 기름 + 불 → 불 + CO2
    registry.registerReaction({
        .reactant_a = OIL,
        .reactant_b = FIRE,
        .handler = react_oil_fire,
        .probability = 0.7f,
        .min_temperature = -999.0f,
        .name = "Oil Combustion"
    });
    
    // 불 + 기름 (순서 반대)
    registry.registerReaction({
        .reactant_a = FIRE,
        .reactant_b = OIL,
        .handler = [](const Particle& f, const Particle& oil, int fx, int fy, int oilx, int oily) {
            return react_oil_fire(oil, f, oilx, oily, fx, fy);
        },
        .probability = 0.7f,
        .min_temperature = -999.0f,
        .name = "Oil Combustion (rev)"
    });
    
    // 수소 + 불 → 불 + 증기 (폭발)
    registry.registerReaction({
        .reactant_a = HYDROGEN,
        .reactant_b = FIRE,
        .handler = react_hydrogen_fire,
        .probability = 0.8f,
        .min_temperature = -999.0f,
        .name = "Hydrogen Explosion"
    });
    
    // 불 + 수소 (순서 반대)
    registry.registerReaction({
        .reactant_a = FIRE,
        .reactant_b = HYDROGEN,
        .handler = [](const Particle& f, const Particle& h, int fx, int fy, int hx, int hy) {
            return react_hydrogen_fire(h, f, hx, hy, fx, fy);
        },
        .probability = 0.8f,
        .min_temperature = -999.0f,
        .name = "Hydrogen Explosion (rev)"
    });
}
