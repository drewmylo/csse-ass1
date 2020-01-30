#include "game.h"
#include "error.h"
#include "utils.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF 512
#define MAX_HAND 6
#define SIDES 4

/**
 * Frees all remaining resources.
 * Parameters:
 *        game Current game state
 */
void clean_up(Game *game) {

    for (int i = 0; i < game->width; i++) {
        free(game->board[i]);
    }
    free(game->board);
    free(game->saveName);
    free(game->deckName);
    for (int x = 0; x < 2; x++) {
        free(game->playerHands[x].playerHand);
    }
    free(game->playerHands);
    free(game->coordinates);
    free(game->parsedDeck);
    free(game->pType);
    fclose(game->deck);
    free(game);
}

/**
 * Switches the current player.
 * Parameters:
 *        game Current game state
 */
void next_player(Game *game) {
    if (game->player == 0) {
        game->player = 1;
    } else {
        game->player = 0;
    }
}

/**
 * Reads and acts upon human input.
 * Parameters:
 *        game Current game state
 *
 */
int parse_input(Game *game) {
    printf("Move? ");
    fflush(stdout);
    char inputBuffer[BUFF];
    if (!(fgets(inputBuffer, BUFF, stdin))) {
        return 2;
    }
    if (inputBuffer[0] == 'S') { //Check for save command.
        if (strncmp(inputBuffer, "SAVE", 4) == 0) {
            if (inputBuffer[4] < 1 ||
                    inputBuffer[4] > 127) {//Ensure there is at least one char.
                printf("Unable to save\n");
                fflush(stdout);

            }
            /*Move the pointer of the input
                                past the command "SAVE".*/
            int size = 0;
            game->saveName = read_line_from_array(&inputBuffer[4], &size);
            if (save_game(game) == 1) {
                printf("Unable to save\n");
                fflush(stdout);
            }
        }
        return 1; //Returns 1 to reprompt for a move.
    } else {
        if (sscanf(inputBuffer, "%d %d %d", &game->coordinates->card,
                &game->coordinates->column, &game->coordinates->row) != 3) {
            return 1;
        }
        game->coordinates->card--;
        game->coordinates->column--;
        game->coordinates->row--;

        if (check_move(game) == 1) {
            return 1;
        }
    }

    return 0;
}

/**
 * Returns the card found at the given coordinates.
 * Parameters:
 *        game Current game state
 *        xY   the given coordinates.
 *
 */
Card *get_card_from_coordinates(Game *game, Coordinates xY) {
    return &game->board[xY.x][xY.y];
}

/**
 * Returns a pointer to a set of coordinates that describe the adjacent sides.
 * Parameters:
 *         game Current game state
 *         centre The coordinates of the card in the centre.
 *
 */
Coordinates *get_adjoining_coordinates(Game *game, Coordinates centre) {
    Coordinates *xY = calloc(SIDES, sizeof(Coordinates));
    //Top side
    if (centre.y == 0) {
        xY[0].x = (centre.x);
        xY[0].y = (game->height - 1); // Top is wrapped to bottom.
    } else {
        xY[0].x = (centre.x);
        xY[0].y = (centre.y - 1);
    }
    //Bottom side
    if (centre.y == (game->height - 1)) {
        xY[1].x = (centre.x);
        xY[1].y = 0; //Bottom wraps around to top.
    } else {
        xY[1].x = (centre.x);
        xY[1].y = (centre.y + 1);
    }
    //Left side
    if (centre.x == 0) {
        xY[2].x = (game->width - 1); // Left wraps to the right.
        xY[2].y = (centre.y);
    } else {
        xY[2].x = (centre.x - 1);
        xY[2].y = (centre.y);
    }
    //Right side
    if (centre.x == game->width - 1) {
        xY[3].x = 0; //Right wraps to the left.
        xY[3].y = (centre.y);
    } else {
        xY[3].x = (centre.x + 1);
        xY[3].y = (centre.y);
    }
    return xY;
}

/**
 * Checks to see if the move inputted is adjacent to a card on the board.
 * Parameters:
 *        game Current game state
 *
 */
bool is_move_adjoining(Game *game) {
    Coordinates *xY;
    Coordinates move;
    move.x = game->coordinates->column;
    move.y = game->coordinates->row;
    xY = get_adjoining_coordinates(game, move);
    for (int i = 0; i < SIDES; i++) {
        if (get_card_from_coordinates(game, xY[i])->isBlankSpace == false) {
            free(xY);
            return true;
        }
    }
    free(xY);
    return false;
}

/**
 * Ensures the move inputted is a valid and legal move.
 * Parameters:
 *        game Current game state
 *
 */
int check_move(Game *game) {
    if (game->coordinates->column < 0 ||
            game->coordinates->column > game->width) {
        return 1;
    }
    if (game->coordinates->row < 0 || game->coordinates->row > game->height) {
        return 1;
    }
    if (game->coordinates->card >
            game->playerHands[game->player].cardsInHand ||
            game->coordinates->card < 0) {
        return 1;
    }
    if (game->board[(game->coordinates->column)][(game->coordinates->row)]
            .isBlankSpace == false) {
        return 1;
    }
    if (game->firstMoveFlag == false &&
            is_move_adjoining(game) == false) { /*The first move in the game
                need not be adjoining. Will short circuit in that case*/
        return 1;
    }

    return 0;
}

/**
 * Copies the cards from the hand onto the board, and moves the respective
 * card counts forward.
 * Parameters:
 *        game Current game state
 *
 */
int attempt_move(Game *game) {
    if (game->pType[game->player] == 'a') {
        play_move(game);
    }
    copy_card(
            &game->board[(game->coordinates->column)]
            [(game->coordinates->row)],
            &game->playerHands[game->player]
            .playerHand[game->coordinates->card]);

    for (int i = (game->coordinates->card); i < MAX_HAND; i++) {
        copy_card(&game->playerHands[game->player].playerHand[i],
                &game->playerHands[game->player].playerHand[i + 1]);
    } //Moves the cards in the hand down one.
    game->playerHands[game->player].cardsInHand--;
    game->firstMoveFlag = false;
    return 0;
}

/**
 * Prints the players hands to stdout.
 * Parameters:
 *        game Current game state
 */
void show_hand(Game *game) {
    if (game->pType[game->player] == 'h') {
        printf("Hand(%d):", game->player + 1);
    } else {
        printf("Hand:");
    }
    for (int i = 0; i < game->playerHands[game->player].cardsInHand; i++) {
        if (game->playerHands[game->player].playerHand[i].isBlankSpace ==
                false) {
            putchar(' ');
            putchar(game->playerHands[game->player].playerHand[i].number);
            putchar(game->playerHands[game->player].playerHand[i].letter);
        }
    }
    putchar('\n');
    fflush(stdout);
}

/**
 * Copies card from one location in memory to another.
 * Parameters:
 *         dest Destination of the memory copy.
 *         src  Source of the memory copy.
 *
 */
int copy_card(Card *dest, Card *src) {
    memcpy(dest, src, sizeof(Card));
    return 0;
}

/**
 * Copies a card from the deck into a players hand. Moves respecive card counts
 * forward. Parameters: game Current game state
 *
 */
int draw_card(Game *game) {
    if (game->drawnCards < game->deckSize) {
        game->playerHands[game->player].cardsInHand++;
        Card *cardLocation =
                &game->playerHands[game->player]
                .playerHand[game->playerHands[game->player].cardsInHand - 1];
        copy_card(cardLocation, &game->parsedDeck[game->drawnCards]);
        game->drawnCards++;
    }

    return 0;
}

/**
 * Ensures the player types are valid.
 * Parameters:
 *        game Current game state
 *
 */
int check_player_parameters(Game *game) {
    if (check_player_type(game->pType[0]) == 1) {
        return 1;
    } else if (check_player_type(game->pType[1]) == 1) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Ensures the board dimensions are valid.
 * Parameters:
 *        game Current game state
 *
 */
int check_game_parameters(Game *game) {
    if (check_board_size(game->width) == 1) {
        return 1;
    } else if (check_board_size(game->height) == 1) {
        return 1;
    } else if (game->drawnCards > game->deckSize) {
        return 1;
    } else if (game->drawnCards < 0) {
        return 1;
    } else if (game->player != 1 && game->player != 0) {
        return 1;
    }
    return 0;

}

/**
 * Allocates the memory for a new game board, using supplied dimensions.
 * Parameters:
 *        game Current game state
 *
 */
int create_board(Game *game) {
    game->board = calloc(game->width, sizeof(Card *));
    if (game->board == 0) {
        return 1;
    }

    for (int i = 0; i < game->width; i++) {
        game->board[i] = calloc(game->height, sizeof(Card));
        if (game->board[i] == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Sets every card in the board to a "blank card"
 * Parameters:
 *        game Current game state
 */
void initialise_board(Game *game) {
    for (int i = 0; i < game->width; i++) {
        for (int x = 0; x < game->height; x++) {
            game->board[i][x].isBlankSpace = true;
        }
    }
}

/**
 * Initialises the players hands.
 * Parameters:
 *        game Current game state
 */
void initialise_hands(Game *game) {
    game->player = 0;
    game->playerHands = calloc(2, sizeof(Hand));
    for (int x = 0; x < 2; x++) {
        game->playerHands[x].cardsInHand = 0;
        game->playerHands[x].playerHand = calloc(6, sizeof(Card));
    }
}

/**
 * Sets first move to card 0 at 0,0.
 * Parameters:
 *        game Current game state
 */
void initialise_moves(Game *game) {
    game->firstMoveFlag = true;
    game->coordinates = calloc(1, sizeof(Move));
    game->coordinates->row = 0;
    game->coordinates->column = 0;
    game->coordinates->card = 0;
}

/**
 * Prints the game board to stdout or to a savefile.
 * Parameters:
 *        game      Current game state
 *        save      If true, the board is printed to a savefile.
 *        direction Either stdout or the savefile to save to.
 */
void print_board(Game *game, bool save, FILE *direction) {
    for (int i = 0; i < game->height; i++) {
        for (int x = 0; x < game->width; x++) {
            if (game->board[x][i].isBlankSpace == true) {
                if (save == true) {
                    fputc('*', direction);
                    fputc('*', direction);
                } else {
                    fputc('.', direction); // print blanks
                    fputc('.', direction); // print blanks
                }
            } else {
                fputc(game->board[x][i].number, direction);// print cards
                fputc(game->board[x][i].letter, direction);// print cards

            }
        }
        fputc('\n', direction);
    }
}

/**
 * Writes game in the specified save file.
 * Parameters:
 *        game Current game state
 *
 */
int save_game(Game *game) {
    FILE *saveFile = fopen(game->saveName, "w");
    if (saveFile == NULL) {
        return 1;
    }
    fprintf(saveFile, "%d %d %d %d\n", game->width, game->height,
            game->drawnCards, (game->player + 1));
    fprintf(saveFile, "%s\n", game->deckName);
    for (int x = 0; x < 2; x++) {
        for (int i = 0; i < game->playerHands[x].cardsInHand; i++) {
            fprintf(saveFile, "%c%c",
                    game->playerHands[x].playerHand[i].number,
                    game->playerHands[x].playerHand[i].letter);
        }
        fprintf(saveFile, "\n");
    }

    print_board(game, true, saveFile);
    fclose(saveFile);
    return 0;
}

/**
 * Ensures the size of the deck is larger than the minimum of 11.
 * Parameters:
 *         deckSize The number of cards in the deck
 */
int check_deck_length(int deckSize) {
    if (deckSize < 11) {
        return 1;
    }
    return 0;
}

/**
 * Ensures card is valid.
 * Parameters:
 *        card Card to check
 *
 */
bool is_card_valid(Card card) {
    if (card.number < '1' ||
            card.number > '9') {
        return false;
    }
    if (card.letter < 'A' ||
            card.letter > 'Z') {
        return false;
    }
    return true;

}

/**
 * Reads the deckfile into the appropriate format.
 * Parameters:
 *        game Current game state
 *
 */
int parse_deckfile(Game *game) {
    if (!game->deck) {
        return BAD_DECK;
    }
    if (fscanf(game->deck, "%d\n", &game->deckSize) == 0) {
        return BAD_DECK;
    }
    if (check_deck_length(game->deckSize) == 1) {
        return SHORT_DECK;
    }
    game->parsedDeck = calloc(game->deckSize, sizeof(Card));
    for (int i = 0; i < game->deckSize; i++) {
        game->parsedDeck[i].isBlankSpace = false;
        game->parsedDeck[i].number = fgetc(game->deck);
        game->parsedDeck[i].letter = fgetc(game->deck);
        game->parsedDeck[i].parity = get_parity(game->parsedDeck[i].letter);
        if (is_card_valid(game->parsedDeck[i]) == false) {
            return BAD_DECK;
        }
        if (i != (game->deckSize) - 1) {
            char newline = fgetc(game->deck);
            if (newline != '\n') {
                return BAD_DECK;
            }
        }
    }
    return NORMAL;
    fclose(game->deck);
}

/**
 * Prints the valid move to stdout.
 * Parameters:
 *        game Current game state
 *
 */
int play_move(Game *game) {
    printf("Player %d ", (game->player + 1));
    printf("plays %c%c ",
            game->playerHands[game->player]
            .playerHand[game->coordinates->card]
            .number,
            game->playerHands[game->player]
            .playerHand[game->coordinates->card]
            .letter);
    printf("in column %d ", (game->coordinates->column + 1));
    printf("row %d\n", (game->coordinates->row + 1));
    fflush(stdout);
    return 0;
}

/**
 * Decides the ai's next move.
 * Parameters:
 *        game Current game state
 *
 */
int generate_ai_move(Game *game) {
    //If automatic players are starting, plays first move in centre.
    if (game->firstMoveFlag == true) {
        if (game->height % 2 == 0) {
            game->coordinates->row = ((game->height / 2) - 1);
        } else {
            game->coordinates->row = ((game->height / 2));
        }
        if (game->width % 2 == 0) {
            game->coordinates->column = ((game->width / 2) - 1);
        } else {
            game->coordinates->column = ((game->width / 2));
        }
        game->coordinates->card = 0;
        if (check_move(game) == 0) {
            attempt_move(game);
            return 0;
        }
    }

    if (game->player == 0) { //Player 1's automatic moves.
        for (int i = 0; i < game->height; i++) {
            for (int x = 0; x < game->width; x++) {
                game->coordinates->row = i;
                game->coordinates->column = x;
                game->coordinates->card = 0;
                if (check_move(game) == 0) {
                    attempt_move(game);
                    return 0;
                }
            }
        }
    } else { //Player 2 automatic moves.
        for (int i = (game->height - 1); i >= 0; i--) {
            for (int x = (game->width - 1); x >= 0; x--) {
                game->coordinates->row = i;
                game->coordinates->column = x;
                game->coordinates->card = 0;
                if (check_move(game) == 0) {
                    attempt_move(game);
                    return 0;
                }
            }
        }
    }

    return 1;
}

/**
 * Determines whether there are any moves still playable.
 * Parameters:
 *        game Current game state
 *
 */
bool is_board_empty(Game *game) {
    bool emptySpace = true;
    for (int i = 0; i < game->height; i++) {
        for (int x = 0; x < game->width; x++) {
            if (game->board[x][i].isBlankSpace == false) {
                emptySpace = false;
            }
        }
    }
    if (emptySpace == false) {
        return false;
    }
    return true;
}

/**
 * Determines whether there are any moves still playable.
 * Parameters:
 *        game Current game state
 *
 */
int check_end(Game *game) {
    bool emptySpace = false;
    if (game->drawnCards >= game->deckSize) {
        return 2;
    }
    for (int i = 0; i < game->height; i++) {
        for (int x = 0; x < game->width; x++) {
            if (game->board[x][i].isBlankSpace == true) {
                emptySpace = true;
            }
        }
    }
    if (emptySpace == false) {
        return 1;
    }
    return 0;
}

/**
 * Determines the longest path recursively.
 * Parameters:
 *         xY                       The coordinates of the current step in the
 * path.
 * originalCharacter             The character that started the path.
 * distance The distance from the starting card.
 * distanceFromHighestSoFar Distance between two cards of the same suit so far.
 * game                     Current game state.
 */
int get_path(Coordinates xY, char originalCharacter, int distance,
        int distanceFromHighestSoFar, Game *game) {
    distance++;
    if (get_card_from_coordinates(game, xY)->letter == originalCharacter) {
        distanceFromHighestSoFar = distance;
    }
    int temp = 0;
    Coordinates *xYSurrounding = get_adjoining_coordinates(game, xY);
    Card cardsWithPath[SIDES] = {{0}};
    for (int i = 0; i < SIDES; i++) {
        copy_card(&cardsWithPath[i],
                get_card_from_coordinates(game, xYSurrounding[i]));
        cardsWithPath[i].hasPath = false;
        if ((cardsWithPath[i].number >
                get_card_from_coordinates(game, xY)->number) &&
                cardsWithPath[i].isBlankSpace == false) {
            cardsWithPath[i].hasPath = true;
        }
    }
    for (int x = 0; x < SIDES; x++) {
        if (cardsWithPath[x].hasPath == true) {
            temp = get_path(xYSurrounding[x], originalCharacter, distance,
                    distanceFromHighestSoFar, game);
            if (temp > distanceFromHighestSoFar) {
                distanceFromHighestSoFar = temp;
            }
        }
    }
    free(xYSurrounding);
    return distanceFromHighestSoFar;
}

/**
 * Finds the longest path possible for each card on the board.
 * Parameters:
 *         game Current game state
 *         parity Determines whether to count odd or even cards
 *
 */
int longest_path(Game *game, int parity) {
    int currentLongest = 0;
    int placeHolder = 0;
    for (int i = 0; i < game->width; i++) {
        for (int x = 0; x < game->height; x++) {
            if (game->board[i][x].isBlankSpace == false &&
                    (game->board[i][x].parity == parity)) {
                Coordinates startingCoordinates;
                startingCoordinates.x = i;
                startingCoordinates.y = x;
                placeHolder = get_path(
                        startingCoordinates,
                        get_card_from_coordinates(game, startingCoordinates)
                        ->letter,
                        0, 0, game);
                if (placeHolder > currentLongest) {
                    currentLongest = placeHolder;
                }
            }
        }
    }
    return currentLongest;
}

/**
 * Creates a card from a a valid string.
 * Parameters:
 *         rawChars The charactes to read a card from.
 */
Card *card_from_string(char *rawChars) {
    Card *card = calloc(1, sizeof(Card));
    if (rawChars[0] == '*') {
        card->isBlankSpace = true;
        return card;
    } else {
        card->number = rawChars[0];
        card->letter = rawChars[1];
        card->parity = get_parity(card->letter);
        card->isBlankSpace = false;
        return card;
    }
}

/**
 * Reads a savefile into the appropriate locations in memory.
 * Parameters:
 *         game     Current game state
 *         saveFile The file to read from.
 *
 */
int parse_savefile(Game *game, FILE *saveFile) {
    if (!saveFile) {
        return 1;
    }
    if (fscanf(saveFile, "%d %d %d %d\n", &game->width, &game->height,
            &game->drawnCards, &game->player) != 4) {
        return 1;
    }
    game->player--;
    game->fromSave = true;
    int readSize = 0;
    game->deckName = read_line(saveFile, &readSize);
    for (int z = 0; z < 2; z++) {
        char *readHand = read_line(saveFile, &readSize);
        for (int i = 0; i < ((readSize - 1) / 2);
                i++) { // readsize -1 0 and divide by two as each card has two
            // chars.
            if (check_for_valid_card(&readHand[(i * 2)]) == 1) {
                return 1;
            }
            Card *readCard = card_from_string(&readHand[(i * 2)]);
            copy_card(&game->playerHands[z].playerHand[i], readCard);
            if (is_card_valid(game->playerHands[z].playerHand[i]) == false) {
                return 1;
            }
            free(readCard);
        }
        game->playerHands[z].cardsInHand = ((readSize - 1) / 2);
    }
    create_board(game); // create space for new board
    initialise_board(game);
    // read board
    for (int i = 0; i < game->height; i++) {
        char *readHand = read_line(saveFile, &readSize);
        for (int x = 0; x < game->width; x++) {
            if (check_for_valid_card(&readHand[(x * 2)]) == 1) {
                return 1;
            }
            Card *readCard = card_from_string(&readHand[(x * 2)]);
            copy_card(&game->board[x][i], readCard);
            free(readCard);
        }
    }
    fclose(saveFile);
    return 0;
}

/**
 * Sets up and initialises a new game state
 * Parameters:
 *            game Current game state
 *            argc argument count from main()
 *            argv array of arguments from main()
 *
 */
Error begin_new_game(Game *game, int argc, char const** argv) {
    game->fromSave = false;
    game->width = atoi(argv[2]);  // parse width
    game->height = atoi(argv[3]); // parse height
    game->pType[0] = argv[4][0];  // parse player1 type
    game->pType[1] = argv[5][0];  // parse player2 type
    if (check_player_parameters(game) == 1) {
        return BAD_ARG_TYPE;
    }
    if (check_game_parameters(game) == 1) {
        return BAD_ARG_TYPE;
    }
    game->deckName = malloc(sizeof(char) * (sizeof(argv[1])));
    strcpy(game->deckName, argv[1]);
    game->deck = fopen(argv[1], "r");
    // open deckfile
    Error errorCode = parse_deckfile(game);
    if (errorCode != NORMAL) { // check result
        return errorCode;
    }
    create_board(game);     // create space for new board
    initialise_board(game); // initialise the empty spaces
    initialise_hands(game);
    initialise_moves(game);

    game->drawnCards = 0;
    for (int i = 0; i < 5; i++) {
        draw_card(game);
    }
    next_player(game);
    for (int i = 0; i < 5; i++) {
        draw_card(game);
    }
    return NORMAL;
}

/**
 * Sets up the necessary flags to begin from savefile.
 * Parameters:
 *            game Current game state
 *            argc argument count from main()
 *            argv array of arguments from main()
 *
 */
Error begin_from_save(Game *game, int argc, char const** argv) {
    FILE *saveFile = fopen(argv[1], "r");
    game->pType[0] = argv[2][0]; // parse player1 type
    game->pType[1] = argv[3][0]; // parse player2 type
    if (check_player_parameters(game) == 1) {
        return BAD_ARG_TYPE;
    }
    initialise_hands(game);
    initialise_moves(game);
    if (parse_savefile(game, saveFile) == 1) {
        return BAD_SAVE;
    }
    if (is_board_empty(game) == true) {
        game->firstMoveFlag = true;
    } else {
        game->firstMoveFlag = false;
    }
    game->deck = fopen(game->deckName, "r"); // open deckfile
    if (parse_deckfile(game) != 0) {         // check result
        return BAD_DECK;
    }
    if (check_game_parameters(game) == 1) {
        return BAD_SAVE;
    }
    if (check_end(game) == 1) {
        return FULL_BOARD;
    }

    return NORMAL;
}

/**
 * Runs the game until there are no more cards in the deck or no more valid
 * moves
 * Parameters:
 *            game Current game state
 *
 */
Error run_game(Game *game) {
    print_board(game, false, stdout); // display board

    if (check_end(game) != 0) {

        printf("Player 1=%d Player 2=%d\n", longest_path(game, 1),
                longest_path(game, 2));
        fflush(stdout);
        clean_up(game);
        return GAME_OVER;
    }
    if (game->fromSave == false) {
        next_player(game);
        draw_card(game);
    }
    game->fromSave = false;
    show_hand(game);
    if (game->pType[game->player] == 'a') {
        generate_ai_move(game);
    } else {
        int inputResult = parse_input(game);
        while (inputResult == 1) {
            inputResult = parse_input(game); //reprompt
        }
        if (inputResult == 2) {
            return INPUT_END; //CTRL-D
        }
        attempt_move(game); //Play move
    }
    return NORMAL;
}
