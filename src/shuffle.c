#include "shuffle.h"

#include <assert.h>
#include <stdio.h>

void shuffle(Shuffler *shuffler) {
  for (uint8_t i = 0; i < shuffler->size; i++) {
    shuffler->order[i] = i;
  }
  for (uint8_t i = 0; i < shuffler->size; i++) {
    uint8_t j = rand() % shuffler->size;
    uint8_t temp = shuffler->order[i];
    shuffler->order[i] = shuffler->order[j];
    shuffler->order[j] = temp;
  }
}

void initShuffler(Shuffler *shuffler, Tetramino *tetraminoes, uint8_t size) {
  assert(shuffler != NULL);
  assert(tetraminoes != NULL);
  assert(size > 0);
  shuffler->tetraminoes = tetraminoes;
  shuffler->size = size;
  shuffler->index = 0;
  shuffler->order = malloc(sizeof(uint8_t) * size);
  shuffle(shuffler);
}

void freeShuffler(Shuffler *shuffler) {
  assert(shuffler != NULL);
  free(shuffler->order);
  free(shuffler);
}

void nextTetramino(Shuffler *shuffler, TetraminoInstance *tetramino) {
  assert(shuffler != NULL);
  assert(tetramino != NULL);
  if (shuffler->index == shuffler->size) {
    shuffle(shuffler);
    shuffler->index = 0;
  }

  assert(tetramino != NULL);
  tetramino->rotation = 0;
  tetramino->x = 3;
  tetramino->y = 20;
  tetramino->tetramino =
      shuffler->tetraminoes[shuffler->order[shuffler->index]];
  shuffler->index++;
}