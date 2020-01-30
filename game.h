
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdbool.h>
#include "error.h"

typedef struct {
    int x;
    int y;
} Coordinates;

typedef struct {
    char number;
    char letter;
    int parity;
    bool isBlankSpace;
    bool hasPath;
} Card;

typedef struct {
    Card *playerHand;
    int cardsInHand;

} Hand;

typedef struct {
    int card;
    int column;
    int row;
} Move;

typedef struct {
    int player;
    bool fromSave;
    int width;
    int height;
    int drawnCards;
    char *pType;
    bool firstMoveFlag;
    char *deckName;
    char *saveName;
    FILE *deck;
    Card *parsedDeck;
    int deckSize;
    Hand *playerHands;
    Card **board;
    Move *coordinates;

} Game;
int check_player_parameters(Game *game);
int check_end(Game *game);
int play_move(Game *game);
void clean_up(Game *game);
int generate_ai_move(Game *game);
void initialise_moves(Game *game);
int check_move(Game *game);
int copy_card(Card *dest, Card *src);
int attempt_move(Game *game);
int parse_input(Game *game);
void initialise_hands(Game *game);
int create_board(Game *game);
void initialise_board(Game *game);
int draw_card(Game *game);
int save_game(Game *game);
void show_hand(Game *game);
void print_board(Game *game, bool save, FILE *direction);
void next_player(Game *game);
int parse_deckfile(Game *game);
int parse_savefile(Game *game, FILE *saveFile);
int check_game_parameters(Game *game);
int longest_path(Game *game, int parity);
Error begin_new_game(Game *game, int argc, char const* argv[]);
Error begin_from_save(Game *game, int argc, char const* argv[]);
Error run_game(Game *game);
#endif
