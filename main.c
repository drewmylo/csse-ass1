#include "error.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Checks the arguments supplied to either create and
*new game or continue from a
*saved file.
* Parameters:
*             argc Argument count
*             argv Array of arguments passed
*
 */
int main(int argc, char const *argv[]) {
    Game *game = calloc(1, sizeof(Game));
    game->pType = calloc(2, sizeof(char));
    if (argc != 6 && argc != 4) {
        return exit_with_error(BAD_ARGS_NUM);
    }
    if (argc == 6) {
        Error errorCode = begin_new_game(game, argc, argv);
        if (errorCode != NORMAL) {
            return exit_with_error(errorCode);
        }

    } else if (argc == 4) {
        Error errorCode = begin_from_save(game, argc, argv);
        if (errorCode != NORMAL) {
            return exit_with_error(errorCode);
        }
    } else {
        return exit_with_error(BAD_ARG_TYPE);
    }
    while (1) {
        Error errorCode = run_game(game);
        if (errorCode == GAME_OVER) {
            return 0;
        } else if (errorCode != NORMAL) {
            return exit_with_error(errorCode);
        }
    }

    return 0;
}
