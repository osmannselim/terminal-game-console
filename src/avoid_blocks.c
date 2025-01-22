#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>

#define ROWS 15
#define COLS 7

char grid[ROWS][COLS];
int player_pos = COLS / 2;
int blocks[COLS];
int score = 0;
int block_fall_delay = 0;

struct termios orig_termios;

// Reset terminal on exit
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    system("clear");
}

// Signal handler for graceful exit
void handle_signal(int sig) {
    reset_terminal();
    printf("\nGame over! Your final score: %d\n", score);
    exit(0);
}

// Set terminal to non-canonical mode for real-time input
void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

// Non-blocking input
int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

// Initialize grid and game state
void initialize_grid() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = '.';
        }
    }
    for (int j = 0; j < COLS; j++) {
        blocks[j] = -1; // No blocks initially
    }
    grid[ROWS - 1][player_pos] = 'O'; // Player's initial position
}

// Display grid
void display_grid() {
    system("clear");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
    printf("Score: %d\n", score);
}

// Move the player left or right
void move_player(char direction) {
    grid[ROWS - 1][player_pos] = '.';
    if (direction == 'a' && player_pos > 0) {
        player_pos--;
    } else if (direction == 'd' && player_pos < COLS - 1) {
        player_pos++;
    }
    grid[ROWS - 1][player_pos] = 'O';
}

// Spawn new blocks randomly at the top
void spawn_blocks() {
    for (int i = 0; i < COLS; i++) {
        if (rand() % 10 == 0) { // 10% chance for a block to spawn in each column
            if (blocks[i] == -1) {
                blocks[i] = 0;   // Mark block's initial row
                grid[0][i] = '#'; // Place block at the top
            }
        }
    }
}

// Update blocks' positions and check collisions
void update_blocks() {
    for (int i = ROWS - 2; i >= 0; i--) { // Start from the bottom-most row
        for (int j = 0; j < COLS; j++) {
            if (grid[i][j] == '#') {
                grid[i][j] = '.'; // Clear current position
                if (i + 1 < ROWS - 1) {
                    grid[i + 1][j] = '#'; // Move block down
                    blocks[j] = i + 1;    // Update block's position
                } else if (i + 1 == ROWS - 1) { // Block reaches the player's row
                    if (j == player_pos) {
                        handle_signal(0); // Game over if block hits player
                    } else {
                        blocks[j] = -1; // Clear block when it reaches the bottom
                    }
                }
            }
        }
    }
}

// Main game function
int main() {
    printf("%c]0;%s%c", '\033', "avoid_blocks", '\007');
    srand(time(NULL));
    setup_terminal();
    initialize_grid();

    while (1) {
        display_grid();

        if (kbhit()) {
            char input = getchar();
            if (input == 'q') break; // Quit the game
            if (input == 'a' || input == 'd') {
                move_player(input); // Move the player
            }
        }

        block_fall_delay++;
        if (block_fall_delay >= 5) { // Delay block movement to make it manageable
            update_blocks();
            spawn_blocks();
            score++; // Increment score as time progresses
            block_fall_delay = 0; // Reset delay
        }

        usleep(100000); // Control game loop speed
    }

    handle_signal(0);
    return 0;
}