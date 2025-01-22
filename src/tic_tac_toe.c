#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // For read()
#include <termios.h> // For terminal settings
#include <ctype.h>   // For tolower()

// Function Prototypes
void display_board();
int check_winner();
void player_move();
void reset_game();
void setup_terminal();
void reset_terminal();
void display_game_over_screen(int result);

// Global variables
char board[3][3];
char current_player = 'X';
struct termios orig_termios;

int main() {
    int game_over = 0;

    printf("%c]0;%s%c", '\033', "tic_tac_toe", '\007');

    printf("Welcome to Tic Tac Toe!\n");
    reset_game();
    setup_terminal();

    while (!game_over) {
        display_board();
        player_move();

        game_over = check_winner();

        if (game_over == 1 || game_over == -1) {
            display_game_over_screen(game_over);
            break;
        }

        // Switch player
        current_player = (current_player == 'X') ? 'O' : 'X';
    }

    reset_terminal();
    return 0;
}

// Function to reset the game board
void reset_game() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = '1' + (i * 3 + j);  // Initialize with cell numbers (1-9)
        }
    }
}

// Function to display the current state of the board
void display_board() {
    system("clear");
    printf("Tic Tac Toe\n");
    printf("Player X  -  Player O\n\n");

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("---|---|---\n");
    }
    printf("Enter your move (1-9) or 'q' to quit: \n");
}

// Function to display game over screen
void display_game_over_screen(int result) {
    system("clear");
    printf("Game Over\n");
    printf("==========\n\n");

    if (result == 1) {
        // A player has won
        printf("Player %c won!\n\n", current_player);
    } else if (result == -1) {
        // Draw
        printf("Draw!\n\n");
    }

    printf("To exit press 'q'\n");

    // Wait for 'q' to exit
    char input;
    while (1) {
        if (read(STDIN_FILENO, &input, 1) == -1) {
            perror("Error reading input");
            exit(1);
        }

        if (tolower(input) == 'q') {
            break;
        }
    }
}

// Function to check for a winner or a draw
int check_winner() {
    // Check rows and columns
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) return 1;
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) return 1;
    }

    // Check diagonals
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) return 1;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) return 1;

    // Check for a draw
    int is_draw = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != 'X' && board[i][j] != 'O') {
                is_draw = 0;
                break;
            }
        }
        if (!is_draw) break;
    }
    if (is_draw) return -1;

    // No winner yet
    return 0;
}

// Function to handle player moves
void player_move() {
    char input;
    int valid_move = 0;

    while (!valid_move) {

        // Use read to get immediate input
        if (read(STDIN_FILENO, &input, 1) == -1) {
            perror("Error reading input");
            exit(1);
        }

        // Handle quit input
        if (tolower(input) == 'q') {
            printf("\nGame terminated by the player. Goodbye!\n");
            reset_terminal();
            exit(0);  // Terminate the program immediately
        }

        // Validate numeric input
        int move = input - '0';
        if (move < 1 || move > 9) {
            printf("\nInvalid move. Please try again.\n");
        } else {
            int row = (move - 1) / 3;
            int col = (move - 1) % 3;

            if (board[row][col] != 'X' && board[row][col] != 'O') {
                board[row][col] = current_player;
                valid_move = 1;
            } else {
                printf("\nCell already occupied. Please try again.\n");
            }
        }
    }
}

// Setup terminal for non-canonical input
void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // Get original terminal settings
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios); // Apply new terminal settings
}

// Reset terminal to its original settings
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore original terminal settings
    system("clear");
}