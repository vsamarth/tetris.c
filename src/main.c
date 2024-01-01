#include "raylib.h"

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
const Color LINE_COLOR = {44, 44, 44, 255};

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
}

int main(int argc, char *argv[]) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    RenderGrid();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}