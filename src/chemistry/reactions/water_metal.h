#ifndef WATER_METAL_H
#define WATER_METAL_H

#include "../reaction_registry.h"

// 물-금속 반응 등록
void registerWaterMetalReactions(ReactionRegistry& registry);

// 개별 반응 핸들러
ReactionResult react_water_lithium(const Particle& water, const Particle& lithium, int wx, int wy, int lx, int ly);
ReactionResult react_water_sodium(const Particle& water, const Particle& sodium, int wx, int wy, int sx, int sy);

#endif // WATER_METAL_H
