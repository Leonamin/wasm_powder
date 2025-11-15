#include "evaporation.h"
#include "../reaction_system.h"

// 유증기 + 불 → 불 + CO2 (유증기도 연소 가능)
ReactionResult react_oil_steam_fire(const Particle& oil_steam, const Particle& fire, int sx, int sy, int fx, int fy) {
    ReactionResult result;
    
    // 확률 80% (기름보다 더 잘 탐 - 기체 상태)
    if (randomFloat() > 0.8f) {
        return result;
    }
    
    // 반응 발생 - 유증기 연소
    result.occurred = true;
    result.new_type_center = FIRE;       // 유증기 → 불
    result.new_type_neighbor = CO2;      // 불 → CO2
    result.heat_released = 35000.0f;     // 강한 발열 (기름보다 약간 강함)
    result.life_center = 35 + rand() % 35;  // 불 수명: 35~70 프레임
    result.life_neighbor = -1;           // CO2는 무한
    
    return result;
}

// 증발/응축 반응 등록
void registerEvaporationReactions(ReactionRegistry& registry) {
    // 유증기 + 불 → 불 + CO2
    registry.registerReaction({
        .reactant_a = STEAM_OIL,
        .reactant_b = FIRE,
        .handler = react_oil_steam_fire,
        .probability = 0.8f,
        .min_temperature = -999.0f,
        .name = "Oil Steam Combustion"
    });
    
    // 불 + 유증기 (순서 반대)
    registry.registerReaction({
        .reactant_a = FIRE,
        .reactant_b = STEAM_OIL,
        .handler = [](const Particle& f, const Particle& s, int fx, int fy, int sx, int sy) {
            return react_oil_steam_fire(s, f, sx, sy, fx, fy);
        },
        .probability = 0.8f,
        .min_temperature = -999.0f,
        .name = "Oil Steam Combustion (rev)"
    });
}
