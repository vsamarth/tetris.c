#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

void RenderCell(int x, int y, Color color);

typedef enum { CELL_EMPTY, CELL_CYAN } CellState;

typedef enum { ACTION_NONE, ACTION_QUIT, ACTION_AUTODROP } Action;

typedef struct {
  double lastTick;
  double duration;
} Timer;

typedef struct {
  uint32_t rotations[4];
  Color color;
} Tetramino;

typedef struct {
  Tetramino tetramino;
  int rotation;
  int x;
  int y;
} TetraminoInstance;

/*
 * Constants
 */
const int CELL_SIZE = 40;
const int PLAYFIELD_HIDDEN_ROWS = 40;
const int PLAYFIELD_ROWS = 20;
const int PLAYFIELD_COLS = 10;
const int PADDING_X = 12;
const int PADDING_Y = 22;

const int SCREEN_WIDTH = CELL_SIZE * PLAYFIELD_COLS + 2 * PADDING_X;
const int SCREEN_HEIGHT = CELL_SIZE * PLAYFIELD_ROWS + 2 * PADDING_Y;

const char *WINDOW_TITLE = "Tetris";

const Color BACKGROUND_COLOR = {240, 240, 240, 255};
const Color LINE_COLOR = LIGHTGRAY;
const Color RED_TILE = {0xdc, 0x26, 0x26, 0xff};
const Color CYAN_COLOR = {0x4e, 0x9a, 0xa8, 0xff};

CellState playfield[PLAYFIELD_ROWS][PLAYFIELD_COLS];

Tetramino TETRAMINO_I = {
    .rotations = {0x0F00, 0x2222, 0x00F0, 0x4444},
    .color =  {0x4e, 0x9a, 0xa8, 0xff},
};

TetraminoInstance *incomingTetramino = NULL;

Timer autoDropTimer;

void TimerCreate(Timer *timer, double duration) {
  timer->lastTick = GetTime();
  timer->duration = duration;
}

bool TimerHasElapsed(Timer *timer) {
  double now = GetTime();
  if (now - timer->lastTick >= timer->duration) {
    timer->lastTick = now;
    return true;
  }
  return false;
}

/*
 * User Interface
 */

void RenderGrid() {
  for (int i = 0; i <= PLAYFIELD_ROWS; i++) {
    int y = SCREEN_HEIGHT - PADDING_Y - i * CELL_SIZE;
    DrawLine(PADDING_X, y, SCREEN_WIDTH - PADDING_X, y, LINE_COLOR);
  }

  for (int i = 0; i <= PLAYFIELD_COLS; i++) {
    int x = PADDING_X + i * CELL_SIZE;
    DrawLine(x, PADDING_Y, x, SCREEN_HEIGHT - PADDING_Y, LINE_COLOR);
  }

  for (int i = 0; i < PLAYFIELD_ROWS; i++) {
    for (int j = 0; j < PLAYFIELD_COLS; j++) {
      if (playfield[i][j] == CELL_CYAN) {
        RenderCell(j, i, CYAN_COLOR);
      }
    }
  }
}

void RenderCell(int x, int y, Color color) {
  assert(x >= 0 && x < PLAYFIELD_COLS);
  assert(y >= 0 && y < PLAYFIELD_ROWS);
  int cellX = PADDING_X + x * CELL_SIZE;
  int cellY = SCREEN_HEIGHT - PADDING_Y - (y + 1) * CELL_SIZE;
  Rectangle cell = {cellX + 0.5f, cellY + 0.5f, CELL_SIZE - 0.5f,
                    CELL_SIZE - 0.5f};
  DrawRectangleRec(cell, color);
}

void InitGame() {
  incomingTetramino = malloc(sizeof(TetraminoInstance));
  incomingTetramino->tetramino = TETRAMINO_I;
  incomingTetramino->x = 3;
  incomingTetramino->y = 21;
  incomingTetramino->rotation = 0;

  memset(playfield, 0, sizeof(playfield));

  TimerCreate(&autoDropTimer, 0.5);
}

bool CanRenderTetronimoInstance(TetraminoInstance *instance,
                                uint8_t *render_coords) {
  int i = 0;
  for (int y = 0; y < 4; y++) {
    uint32_t row = instance->tetramino.rotations[instance->rotation] >> (y * 4);
    for (int x = 0; x < 4; x++) {
      if (row & 0x1) {
        uint8_t _x = instance->x + x;
        uint8_t _y = instance->y - y;

        if (_x < 0 || _x >= PLAYFIELD_COLS || _y < 0 || _y >= PLAYFIELD_ROWS ||
            playfield[_y][_x] != CELL_EMPTY) {
          return false;
        }

        render_coords[i++] = _x;
        render_coords[i++] = _y;
      }
      row >>= 1;
    }
  }
  assert(i == 8);
  return true;
}

void GetCoordinates(TetraminoInstance *instance, uint8_t *coords) {
  int i = 0;
  for (int y = 0; y < 4; y++) {
    uint32_t row = instance->tetramino.rotations[instance->rotation] >> (y * 4);
    for (int x = 0; x < 4; x++) {
      if (row & 0x1) {
        uint8_t _x = instance->x + x;
        uint8_t _y = instance->y - y;
        coords[i++] = _x;
        coords[i++] = _y;
      }
      row >>= 1;
    }
  }
  assert(i == 8);
}

void RenderTetrominoInstance(TetraminoInstance *instance) {
  for (int y = 0; y < 4; y++) {
    uint32_t row = instance->tetramino.rotations[instance->rotation] >> (y * 4);
    for (int x = 0; x < 4; x++) {
      if (row & 0x1) {
        RenderCell(instance->x + x, instance->y - y, instance->tetramino.color);
      }
      row >>= 1;
    }
  }
}

void RenderGame() { RenderTetrominoInstance(incomingTetramino); }

/*
 * Input
 */
Action HandleInput() {
  if (TimerHasElapsed(&autoDropTimer)) {
    return ACTION_AUTODROP;
  }
  return ACTION_NONE;
}

void HandleAutoDrop() {
  uint8_t render_coords[8];
  uint8_t coords[8];
  incomingTetramino->y--;
  if (!CanRenderTetronimoInstance(incomingTetramino, render_coords)) {
    incomingTetramino->y++;
    for (int i = 0; i < 8; i += 2) {
      GetCoordinates(incomingTetramino, coords);
      playfield[coords[i + 1]][coords[i]] = CELL_CYAN;
    }
    free(incomingTetramino);
    incomingTetramino = malloc(sizeof(TetraminoInstance));
    incomingTetramino->tetramino = TETRAMINO_I;
    incomingTetramino->x = 3;
    incomingTetramino->y = 21;
    incomingTetramino->rotation = 0;
  }

}

void HandleAction(Action action) {
  switch (action) {
  case ACTION_NONE:
    break;
  case ACTION_QUIT:
  case ACTION_AUTODROP:
    HandleAutoDrop();
  }
}

int main(int argc, char *argv[]) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  InitGame();
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    Action action = HandleInput();
    HandleAction(action);

    RenderGrid();
    RenderGame();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}