#ifndef COMBUSTION_H
#define COMBUSTION_H

#include "../reaction_registry.h"

// 연소 반응 등록
void registerCombustionReactions(ReactionRegistry& registry);

// 개별 반응 핸들러
ReactionResult react_wood_oxygen(const Particle& wood, const Particle& oxygen, int wx, int wy, int ox, int oy);
ReactionResult react_oil_oxygen(const Particle& oil, const Particle& oxygen, int oilx, int oily, int ox, int oy);
ReactionResult react_hydrogen_oxygen(const Particle& hydrogen, const Particle& oxygen, int hx, int hy, int ox, int oy);

#endif // COMBUSTION_H
