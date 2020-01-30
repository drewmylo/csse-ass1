
#ifndef ERROR_H
#define ERROR_H
#include <stdio.h>

typedef enum {
    NORMAL = 0,       // everything ok
    BAD_ARGS_NUM = 1, // wrong number of arguments
    BAD_ARG_TYPE = 2, // wrong type of arguments
    BAD_DECK = 3,     // problem with the dec
    BAD_SAVE = 4,     // problem with the save file
    SHORT_DECK = 5,   // deck too short
    FULL_BOARD = 6,   // loaded board is full
    INPUT_END = 7,    // no more input required
    GAME_OVER = 8,    //end of game
} Error;

const char *error_message(Error error);
Error exit_with_error(Error error);

#endif
