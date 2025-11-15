#ifndef EVAPORATION_H
#define EVAPORATION_H

#include "../reaction_registry.h"

// 증발/응축 반응 등록
void registerEvaporationReactions(ReactionRegistry& registry);

// 개별 반응 핸들러
ReactionResult react_oil_evaporation(const Particle& oil, const Particle& neighbor, int oilx, int oily, int nx, int ny);
ReactionResult react_oil_steam_condensation(const Particle& oil_steam, const Particle& neighbor, int sx, int sy, int nx, int ny);

#endif // EVAPORATION_H
