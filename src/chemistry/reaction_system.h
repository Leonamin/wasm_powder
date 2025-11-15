#ifndef REACTION_SYSTEM_H
#define REACTION_SYSTEM_H

#include "../particle.h"

// 반응 결과 구조체
struct ReactionResult {
    bool occurred;                    // 반응 발생 여부
    int new_type_center;             // 중심 입자의 새 타입 (-1 = 변화 없음)
    int new_type_neighbor;           // 이웃 입자의 새 타입 (-1 = 변화 없음)
    float heat_released;             // 방출된 열 (J) - 양수: 발열, 음수: 흡열
    int explosion_radius;            // 폭발 반경 (0 = 폭발 없음)
    float explosion_force;           // 폭발 강도
    int life_center;                 // 중심 입자의 수명 (-2 = 변화 없음)
    int life_neighbor;               // 이웃 입자의 수명 (-2 = 변화 없음)
    
    // 생성자
    ReactionResult() 
        : occurred(false),
          new_type_center(-1),
          new_type_neighbor(-1),
          heat_released(0.0f),
          explosion_radius(0),
          explosion_force(0.0f),
          life_center(-2),
          life_neighbor(-2) {}
};

// 반응 함수 타입 정의
// 매개변수: (중심 입자, 이웃 입자, 중심 x, 중심 y, 이웃 x, 이웃 y)
typedef ReactionResult (*ReactionFunc)(
    const Particle& center,
    const Particle& neighbor,
    int cx, int cy,
    int nx, int ny
);

// 반응 규칙 구조체 (aggregate type for designated initializers)
struct ReactionRule {
    int reactant_a;                  // 반응물 A 타입
    int reactant_b;                  // 반응물 B 타입
    ReactionFunc handler;            // 반응 처리 함수
    float probability;               // 반응 확률 (0.0~1.0)
    float min_temperature;           // 최소 온도 조건 (°C)
    const char* name;                // 반응 이름 (디버깅용)
};

// 메인 화학 반응 업데이트 함수
// 모든 입자를 순회하며 이웃과의 반응을 체크
void updateChemistry();

// 폭발 효과 적용 헬퍼 함수
void applyExplosion(int cx, int cy, int radius, float force);

// 랜덤 float 생성 (0.0 ~ 1.0)
float randomFloat();

#endif // REACTION_SYSTEM_H
