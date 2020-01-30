#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Reads a line and creates and returns null terminated string.
 * Parameters:
 *           file The file to read a line from.
 *           size Size of he resulting string.
 *
 */
char *read_line(FILE *file, int *size) {
    char readCharacter = 'a';
    (*size) = 0;
    char *line = malloc(sizeof(char));
    while (readCharacter != '\n' && readCharacter != '\0'
            && readCharacter != EOF) {
        (*size)++;
        line = realloc(line, sizeof(char) * ((*size)));
        readCharacter = fgetc(file);
        line[(*size) - 1] = readCharacter;
    }
    (*size)++;
    line[(*size) - 2] = '\0';
    return line;
}

/**
 * Reads a line and creates and returns null terminated string.
 * Parameters:
 *           string The array to read a line from.
 *           size Size of he resulting string.
 *
 */
char *read_line_from_array(char *string, int *size) {
    char readCharacter = 'a';
    (*size) = 0;
    char *line = malloc(sizeof(char));
    while (readCharacter != '\n' && readCharacter != '\0'
            && readCharacter != EOF) {
        (*size)++;
        line = realloc(line, sizeof(char) * ((*size)));
        readCharacter = string[(*size) - 1];
        line[(*size) - 1] = readCharacter;
    }
    (*size)++;
    line[(*size) - 2] = '\0';
    return line;
}

/**
 * Checks two chars for validity. Can be two asterisks or a valid card. Returns
 * 1 if neither are found.s
 * Parameters:
 *         rawChars The chars to read the card from.
 */
int check_for_valid_card(char *rawChars) {
    if (rawChars[0] == '*') {
        if (rawChars[1] != '*') {
            return 1;
        }
        return 0;
    }
    if (isdigit(rawChars[0]) == 0) {
        return 1;
    }
    if (isalpha(rawChars[1]) == 0) {
        return 1;
    }

    return 0;
}

/**
 * Determines parity of the card. 2 for even, 1 for odd.
 * Parameters:
 *           letter The letter to find the parity of.
 */
int get_parity(char letter) {
    if (letter % 2 == 0) {
        return 2;
    } else {
        return 1;
    }
}

/**
 * Checks that board size is valid.
 * Parameters:
 *   axis  length of side of board.
 */
int check_board_size(int axis) {
    if (axis > 100 || axis < 3) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Checks that player types are valid.
 * Parameters:
 *            type Automatic ('a') or Human('h') are types that are
 *                 checked for.
 */
int check_player_type(char type) {
    if (type != 'a' && type != 'h') {
        return 1;
    } else {
        return 0;
    }
}
