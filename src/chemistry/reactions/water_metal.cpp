#include "water_metal.h"
#include "../reaction_system.h"

// 물 + 리튬 → 수소 + 수산화리튬 + 폭발
ReactionResult react_water_lithium(const Particle& water, const Particle& lithium, int wx, int wy, int lx, int ly) {
    ReactionResult result;
    
    // 리튬은 물과 격렬하게 반응 (온도 조건 없음)
    // 확률 80% - 매우 반응성 높음
    if (randomFloat() > 0.8f) {
        return result;
    }
    
    // 반응 발생 - 폭발적 반응!
    result.occurred = true;
    result.new_type_center = HYDROGEN;  // 물 → 수소 기체 발생
    result.new_type_neighbor = FIRE;    // 리튬 → 불 (수산화리튬 + 열)
    result.heat_released = 40000.0f;    // 강한 발열 (40kJ)
    result.explosion_radius = 4;        // 폭발 반경 4칸
    result.explosion_force = 2.5f;      // 강한 폭발력
    result.life_center = -1;            // 수소는 무한
    result.life_neighbor = 25 + rand() % 25;  // 불 수명: 25~50 프레임
    
    return result;
}

// 물 + 나트륨 → 수소 + 수산화나트륨 + 폭발
ReactionResult react_water_sodium(const Particle& water, const Particle& sodium, int wx, int wy, int sx, int sy) {
    ReactionResult result;
    
    // 나트륨도 물과 격렬하게 반응 (리튬보다는 약간 덜함)
    // 확률 75%
    if (randomFloat() > 0.75f) {
        return result;
    }
    
    // 반응 발생 - 폭발적 반응!
    result.occurred = true;
    result.new_type_center = HYDROGEN;  // 물 → 수소 기체 발생
    result.new_type_neighbor = FIRE;    // 나트륨 → 불 (수산화나트륨 + 열)
    result.heat_released = 35000.0f;    // 발열 (35kJ)
    result.explosion_radius = 3;        // 폭발 반경 3칸
    result.explosion_force = 2.0f;      // 폭발력
    result.life_center = -1;            // 수소는 무한
    result.life_neighbor = 20 + rand() % 20;  // 불 수명: 20~40 프레임
    
    return result;
}

// 물-금속 반응 등록
void registerWaterMetalReactions(ReactionRegistry& registry) {
    // 물 + 리튬
    registry.registerReaction({
        .reactant_a = WATER,
        .reactant_b = LITHIUM,
        .handler = react_water_lithium,
        .probability = 0.8f,
        .min_temperature = -999.0f,  // 온도 조건 없음
        .name = "Water-Lithium Reaction"
    });
    
    // 리튬 + 물 (순서 반대)
    registry.registerReaction({
        .reactant_a = LITHIUM,
        .reactant_b = WATER,
        .handler = [](const Particle& l, const Particle& w, int lx, int ly, int wx, int wy) {
            return react_water_lithium(w, l, wx, wy, lx, ly);
        },
        .probability = 0.8f,
        .min_temperature = -999.0f,
        .name = "Water-Lithium Reaction (rev)"
    });
    
    // 물 + 나트륨
    registry.registerReaction({
        .reactant_a = WATER,
        .reactant_b = SODIUM,
        .handler = react_water_sodium,
        .probability = 0.75f,
        .min_temperature = -999.0f,  // 온도 조건 없음
        .name = "Water-Sodium Reaction"
    });
    
    // 나트륨 + 물 (순서 반대)
    registry.registerReaction({
        .reactant_a = SODIUM,
        .reactant_b = WATER,
        .handler = [](const Particle& s, const Particle& w, int sx, int sy, int wx, int wy) {
            return react_water_sodium(w, s, wx, wy, sx, sy);
        },
        .probability = 0.75f,
        .min_temperature = -999.0f,
        .name = "Water-Sodium Reaction (rev)"
    });
}
