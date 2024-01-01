#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

void RenderCell(int x, int y, Color color);

typedef enum { CELL_EMPTY, CELL_CYAN } CellState;

typedef enum {
  ACTION_NONE,
  ACTION_AUTODROP,
  ACTION_LEFT,
  ACTION_RIGHT,
  ACTION_ROTATE
} Action;

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
Color CYAN_COLOR = {0x4e, 0x9a, 0xa8, 0xff};

CellState playfield[PLAYFIELD_ROWS][PLAYFIELD_COLS];

Tetramino TETRAMINO_I = {
    .rotations = {0x0F00, 0x2222, 0x00F0, 0x4444},
    .color = {0x4e, 0x9a, 0xa8, 0xff},
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

  TimerCreate(&autoDropTimer, 0.8);
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

bool CanRenderTetronimoInstance(TetraminoInstance *instance,
                                uint8_t *render_coords) {
  GetCoordinates(instance, render_coords);

  for (int i = 0; i < 8; i += 2) {
    uint8_t x = render_coords[i];
    uint8_t y = render_coords[i + 1];
    if (x < 0 || x >= PLAYFIELD_COLS || y < 0 || y >= PLAYFIELD_ROWS ||
        playfield[y][x] != CELL_EMPTY) {
      return false;
    }
  }

  return true;
}

bool RenderTetrominoInstance(TetraminoInstance *instance) {
  uint8_t render_coords[8];
  if (!CanRenderTetronimoInstance(instance, render_coords)) {
    return false;
  }

  for (int i = 0; i < 8; i += 2) {
    uint8_t x = render_coords[i];
    uint8_t y = render_coords[i + 1];
    playfield[y][x] = CELL_CYAN;
  }

  return true;
}

/*
 * Input
 */
Action HandleInput() {
  if (TimerHasElapsed(&autoDropTimer)) {
    return ACTION_AUTODROP;
  }

  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    return ACTION_ROTATE;
  }

  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
    return ACTION_LEFT;
  }

  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
    return ACTION_RIGHT;
  }

  return ACTION_NONE;
}

void LockTetraminoInstance(TetraminoInstance *instance) {
  uint8_t render_coords[8];
  if (!CanRenderTetronimoInstance(instance, render_coords)) {
    return;
  }

  for (int i = 0; i < 8; i += 2) {
    uint8_t x = render_coords[i];
    uint8_t y = render_coords[i + 1];
    playfield[y][x] = CELL_CYAN;
  }

  free(incomingTetramino);
  incomingTetramino = malloc(sizeof(TetraminoInstance));
  incomingTetramino->tetramino = TETRAMINO_I;
  incomingTetramino->x = 3;
  incomingTetramino->y = 21;
  incomingTetramino->rotation = 0;
}

void HandleAction(Action action) {
  uint8_t current_coords[8] = {0};
  uint8_t render_coords[8] = {0};
  GetCoordinates(incomingTetramino, current_coords);

  TetraminoInstance request = *incomingTetramino;

  switch (action) {
  case ACTION_NONE:
    break;
  case ACTION_ROTATE:
    request.rotation = (request.rotation + 1) % 4;
    break;
  case ACTION_LEFT:
    request.x--;
    break;
  case ACTION_RIGHT:
    request.x++;
    break;
  case ACTION_AUTODROP:
    request.y--;

    break;
  }

  if (memcmp(current_coords, render_coords, sizeof(current_coords)) != 0) {
    for (int i = 0; i < 8; i += 2) {
      uint8_t x = current_coords[i];
      uint8_t y = current_coords[i + 1];
      playfield[y][x] = CELL_EMPTY;
    }
    if (CanRenderTetronimoInstance(&request, render_coords)) {
      incomingTetramino->x = request.x;
      incomingTetramino->y = request.y;
      incomingTetramino->rotation = request.rotation;
    } else if (action == ACTION_AUTODROP) {
      LockTetraminoInstance(incomingTetramino);
    }
  }
}

int main(int argc, char *argv[]) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  InitGame();
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    RenderGrid();

    Action action = HandleInput();
    HandleAction(action);

    RenderTetrominoInstance(incomingTetramino);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}