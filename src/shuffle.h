#ifndef SHUFFLE_H
#define SHUFFLE_H

#include "types.h"

typedef struct Shuffler {
    Tetramino *tetraminoes;
    uint8_t size;
    uint8_t index;
    uint8_t *order;
} Shuffler;

void initShuffler(Shuffler *s, Tetramino *tetraminoes, uint8_t size);
void freeShuffler(Shuffler *shuffler);

void nextTetramino(Shuffler *shuffler, TetraminoInstance *tetramino);

#endif