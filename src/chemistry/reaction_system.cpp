#include "reaction_system.h"
#include "reaction_registry.h"
#include "../core/grid.h"
#include "../material_db.h"
#include <cmath>
#include <cstdlib>

// 랜덤 float 생성 (0.0 ~ 1.0)
float randomFloat() {
    return static_cast<float>(rand()) / RAND_MAX;
}

// 폭발 효과 적용
void applyExplosion(int cx, int cy, int radius, float force) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));
            if (dist > radius || dist < 0.1f) continue;
            
            int x = cx + dx;
            int y = cy + dy;
            if (!inBounds(x, y)) continue;
            
            int idx = getIndex(x, y);
            Particle& p = nextGrid[idx];
            
            // 거리에 반비례하는 힘 적용
            float strength = force * (1.0f - dist / radius);
            
            // 속도 추가 (방사형)
            p.vx += (dx / dist) * strength;
            p.vy += (dy / dist) * strength;
            
            // 열 추가
            p.temperature += strength * 50.0f;
            
            // 고체 파괴 (벽 제외)
            if (p.type == WALL) continue;
            
            if (strength > 0.5f && (p.state == STATE_SOLID || p.state == STATE_POWDER)) {
                // 강한 폭발은 고체를 파괴
                if (randomFloat() < strength * 0.3f) {
                    p.type = EMPTY;
                    p.state = STATE_GAS;
                }
            }
        }
    }
}

// 메인 화학 반응 업데이트
void updateChemistry() {
    ReactionRegistry& registry = ReactionRegistry::getInstance();
    
    // 모든 입자를 순회
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = getIndex(x, y);
            const Particle& center = grid[idx];
            
            // EMPTY는 스킵
            if (center.type == EMPTY) continue;
            
            // 8방향 이웃 체크 (대각선 포함 - 연소 범위 확대)
            const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
            const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
            
            for (int dir = 0; dir < 8; dir++) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                
                if (!inBounds(nx, ny)) continue;
                
                int nidx = getIndex(nx, ny);
                const Particle& neighbor = grid[nidx];
                
                // 이웃도 EMPTY면 스킵
                if (neighbor.type == EMPTY) continue;
                
                // 반응 체크
                ReactionResult result = registry.checkReaction(
                    center, neighbor, x, y, nx, ny
                );
                
                if (result.occurred) {
                    // 중심 입자 변경
                    if (result.new_type_center >= 0) {
                        nextGrid[idx].type = result.new_type_center;
                        const Material& mat = getMaterial(result.new_type_center);
                        nextGrid[idx].state = mat.default_state;
                        
                        // 수명 설정
                        if (result.life_center >= -1) {
                            nextGrid[idx].life = result.life_center;
                        }
                    }
                    
                    // 이웃 입자 변경
                    if (result.new_type_neighbor >= 0) {
                        nextGrid[nidx].type = result.new_type_neighbor;
                        const Material& mat = getMaterial(result.new_type_neighbor);
                        nextGrid[nidx].state = mat.default_state;
                        
                        // 수명 설정
                        if (result.life_neighbor >= -1) {
                            nextGrid[nidx].life = result.life_neighbor;
                        }
                    }
                    
                    // 열 방출
                    if (result.heat_released != 0.0f) {
                        nextGrid[idx].temperature += result.heat_released * 0.001f;
                        nextGrid[nidx].temperature += result.heat_released * 0.001f;
                    }
                    
                    // 폭발 효과
                    if (result.explosion_radius > 0) {
                        applyExplosion(x, y, result.explosion_radius, result.explosion_force);
                    }
                    
                    // 한 번 반응하면 이번 프레임은 종료
                    break;
                }
            }
        }
    }
}
