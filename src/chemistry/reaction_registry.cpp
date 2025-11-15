#include "reaction_registry.h"
#include "reactions/combustion.h"
#include "reactions/water_metal.h"
#include "reactions/evaporation.h"
#include <cstdlib>

// 싱글톤 인스턴스
ReactionRegistry& ReactionRegistry::getInstance() {
    static ReactionRegistry instance;
    return instance;
}

// 반응 등록
void ReactionRegistry::registerReaction(const ReactionRule& rule) {
    reactions.push_back(rule);
}

// 두 입자 간 반응 확인
ReactionResult ReactionRegistry::checkReaction(
    const Particle& p1, const Particle& p2,
    int x1, int y1, int x2, int y2
) {
    ReactionResult result;
    
    // 모든 등록된 반응 규칙을 순회
    for (const ReactionRule& rule : reactions) {
        // 반응물 타입 매칭 확인
        bool match = (p1.type == rule.reactant_a && p2.type == rule.reactant_b);
        
        if (!match) continue;
        
        // 온도 조건 확인 (주석 처리 - 불만 닿아도 반응)
        // if (p1.temperature < rule.min_temperature && 
        //     p2.temperature < rule.min_temperature) {
        //     continue;
        // }
        
        // 확률 체크
        float rand_val = static_cast<float>(rand()) / RAND_MAX;
        if (rand_val > rule.probability) {
            continue;
        }
        
        // 반응 핸들러 호출
        if (rule.handler != nullptr) {
            result = rule.handler(p1, p2, x1, y1, x2, y2);
            
            if (result.occurred) {
                // 반응 발생 시 즉시 반환
                return result;
            }
        }
    }
    
    return result; // occurred = false
}

// 모든 반응 초기화
// 각 반응 모듈에서 등록 함수를 호출
void ReactionRegistry::initializeAllReactions() {
    reactions.clear();
    
    // 연소 반응 등록
    registerCombustionReactions(*this);
    
    // 물-금속 반응 등록
    registerWaterMetalReactions(*this);
    
    // 증발/응축 반응 등록
    registerEvaporationReactions(*this);
}
