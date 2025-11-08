#ifndef TYPES_H
#define TYPES_H

// 그리드 크기
const int WIDTH = 400;
const int HEIGHT = 300;
const int GRID_SIZE = WIDTH * HEIGHT;

// Active Chunks 시스템 (현재 비활성화)
const int CHUNK_SIZE = 16;
const int CHUNK_WIDTH = (WIDTH + CHUNK_SIZE - 1) / CHUNK_SIZE;
const int CHUNK_HEIGHT = (HEIGHT + CHUNK_SIZE - 1) / CHUNK_SIZE;
const int CHUNK_COUNT = CHUNK_WIDTH * CHUNK_HEIGHT;

// 물리 상수
const float GRAVITY = 0.3f;
const float VELOCITY_DAMPING = 0.8f;
const float MAX_VELOCITY_Y = 5.0f;
const float MAX_VELOCITY_X = 3.0f;

// 열 전도 상수
const float HEAT_CONDUCTION_BASE = 0.05f;
const float HEAT_CHANGE_THRESHOLD = 0.1f;

#endif // TYPES_H
