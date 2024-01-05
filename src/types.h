#ifndef TYPES_H
#define TYPES_H

#include <raylib.h>
#include <stdlib.h>

typedef enum {
  CELL_EMPTY,
  CELL_CYAN,
  CELL_BLUE,
  CELL_RED,
  CELL_ORANGE,
  CELL_YELLOW,
  CELL_GREEN,
  CELL_PURPLE
} CellState;

typedef enum {
  ACTION_AUTODROP,
  ACTION_DROP,
  ACTION_HARD_DROP,
  ACTION_LEFT,
  ACTION_NONE,
  ACTION_RIGHT,
  ACTION_ROTATE,
  ACTION_RESTART,
} Action;

typedef struct {
  double lastTick;
  double duration;
} Timer;

typedef struct {
  uint32_t rotations[4];
  Color color;
  uint8_t idx;
} Tetramino;

typedef struct {
  Tetramino tetramino;
  int rotation;
  int x;
  int y;
} TetraminoInstance;

typedef enum GameState {
  GAME_STATE_PLAYING,
  GAME_STATE_GAME_OVER,
  GAME_STATE_PAUSED,
} GameState;

typedef struct Game {
  GameState state;
  TetraminoInstance *currentTetramino;
  uint64_t score;

  Tetramino *tetraminoes;  

} Game;

#endif