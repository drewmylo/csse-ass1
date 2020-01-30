
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

char *read_line(FILE *file, int *size);
int check_for_valid_card(char *rawChars);
int get_parity(char letter);
int check_board_size(int axis);
int check_player_type(char type);
char *read_line_from_array(char *string, int *size);
#endif
