#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

void RenderCell(int x, int y, Color color);
void SpawnTetramino();

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
  uint8_t idx;
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
// Cyan: #06b6d4
#define CYAN                                                                   \
  (Color) { 0x06, 0xb6, 0xd4, 0xff }

// Blue: #2563eb
#define BLUE                                                                   \
  (Color) { 0x25, 0x63, 0xeb, 0xff }

// Orange: #ea580c
#define ORANGE                                                                 \
  (Color) { 0xea, 0x58, 0x0c, 0xff }

// Yellow: #facc15
#define YELLOW                                                                 \
  (Color) { 0xfa, 0xcc, 0x15, 0xff }

// Green: #22c55e
#define GREEN                                                                  \
  (Color) { 0x22, 0xc5, 0x5e, 0xff }

// Purple: #9333ea
#define PURPLE                                                                 \
  (Color) { 0x93, 0x33, 0xea, 0xff }

// Red: #dc2626
#define RED                                                                    \
  (Color) { 0xdc, 0x26, 0x26, 0xff }

Tetramino TETRAMINO_I = {
    .rotations = {0x0F00, 0x2222, 0x00F0, 0x4444},
    .color = CYAN,
    .idx = 0,
};

Tetramino TETRAMINO_O = {
    .rotations = {0x0660, 0x0660, 0x0660, 0x0660},
    .color = YELLOW,
    .idx = 4,
};

Tetramino TETRAMINO_T = {
    .rotations = {0x0E40, 0x4C40, 0x4E00, 0x4640},
    .color = PURPLE,
    .idx = 6,
};

Tetramino TETRAMINO_S = {
    .rotations = {0x06C0, 0x8C40, 0x06C0, 0x8C40},
    .color = GREEN,
    .idx = 5,
};

Tetramino TETRAMINO_Z = {
    .rotations = {0x0C60, 0x4C80, 0x0C60, 0x4C80},
    .color = RED,
    .idx = 2,
};

Tetramino TETRAMINO_J = {
    .rotations = {0x44C0, 0x8E00, 0x6440, 0x0E20},
    .color = BLUE,
    .idx = 1,
};

Tetramino TETRAMINO_L = {
    .rotations = {0x4460, 0x0E80, 0xC440, 0x2E00},
    .color = ORANGE,
    .idx = 3,
};

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

Color tetraminoColors[] = {CYAN, BLUE, RED, ORANGE, YELLOW, GREEN, PURPLE};

CellState playfield[PLAYFIELD_ROWS][PLAYFIELD_COLS];

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
      if (playfield[i][j] != CELL_EMPTY) {
        RenderCell(j, i, tetraminoColors[playfield[i][j] - 1]);
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
  memset(playfield, 0, sizeof(playfield));

  TimerCreate(&autoDropTimer, 0.2);
  SpawnTetramino();
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
    playfield[y][x] = instance->tetramino.idx + 1;
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
    playfield[y][x] = instance->tetramino.idx + 1;
  }

  free(incomingTetramino);
  incomingTetramino = malloc(sizeof(TetraminoInstance));
  SpawnTetramino();
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

/*
 * Shuffle
 */
// TODO: Review and test the randomness introduced by this function
void ShuffleArray(int *array, size_t n) {
  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + GetRandomValue(0, n - i - 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

int *bag = NULL;
uint8_t shuffleIndex = 0;
Tetramino *tetraminos[] = {&TETRAMINO_I, &TETRAMINO_O, &TETRAMINO_T,
                           &TETRAMINO_S, &TETRAMINO_Z, &TETRAMINO_J,
                           &TETRAMINO_L};
                          
void SpawnTetramino() {
  if (bag == NULL || shuffleIndex == 7) {
    if (bag != NULL) {
      free(bag);
    }
    bag = malloc(sizeof(int) * 7);
    for (int i = 0; i < 7; i++) {
      bag[i] = i;
    }
    ShuffleArray(bag, 7);
    shuffleIndex = 0;
  
  }
  int idx = bag[shuffleIndex++];
  incomingTetramino = malloc(sizeof(TetraminoInstance));
  incomingTetramino->tetramino = *tetraminos[idx];
  incomingTetramino->x = 3;
  incomingTetramino->y = 21;
  incomingTetramino->rotation = 0;
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