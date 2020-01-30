#include "error.h"
#include <stdio.h>

/**
 * Returns the corresponding error message.
 *
 * Parameters:
 *          r   :   the code to get the error message for
 */
const char *error_message(Error error) {
    switch (error) {
        case NORMAL:
            return "";
        case BAD_ARGS_NUM:
            return "Usage: bark savefile p1type p2type\nbark deck width "
                    "height "
                    "p1type p2type";
        case BAD_ARG_TYPE:
            return "Incorrect arg types";
        case BAD_DECK:
            return "Unable to parse deckfile";
        case BAD_SAVE:
            return "Unable to parse savefile";
        case SHORT_DECK:
            return "Short deck";
        case FULL_BOARD:
            return "Board full";
        case INPUT_END:
            return "End of input";
        default:
            return "Something went very wrong";
    }
}

/**
 * Prints the correct error string to stderr and returns error code.
 * Parameters:
 *            error Error code to print and return.
 */
Error exit_with_error(Error error) {
    if (error != NORMAL) {
        fprintf(stderr, "%s\n", error_message(error));
    }
    return error;
}
