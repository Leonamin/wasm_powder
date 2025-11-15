#ifndef REACTION_REGISTRY_H
#define REACTION_REGISTRY_H

#include "reaction_system.h"
#include <vector>

// 반응 레지스트리 클래스
// 모든 화학 반응을 등록하고 관리
class ReactionRegistry {
public:
    // 싱글톤 인스턴스 획득
    static ReactionRegistry& getInstance();
    
    // 반응 등록
    void registerReaction(const ReactionRule& rule);
    
    // 두 입자 간 반응 확인 및 실행
    // 반환값: 반응이 발생했으면 ReactionResult, 아니면 occurred=false
    ReactionResult checkReaction(
        const Particle& p1, const Particle& p2,
        int x1, int y1, int x2, int y2
    );
    
    // 모든 반응 초기화 (시뮬레이션 시작 시 호출)
    void initializeAllReactions();
    
    // 등록된 반응 개수 반환
    int getReactionCount() const { return reactions.size(); }
    
private:
    // 싱글톤 패턴
    ReactionRegistry() {}
    ReactionRegistry(const ReactionRegistry&) = delete;
    ReactionRegistry& operator=(const ReactionRegistry&) = delete;
    
    // 반응 규칙 저장소
    std::vector<ReactionRule> reactions;
    
    // 빠른 조회를 위한 해시 키 생성
    // key = (type_a << 16) | type_b
    int makeKey(int type_a, int type_b) const {
        return (type_a << 16) | type_b;
    }
};

#endif // REACTION_REGISTRY_H
