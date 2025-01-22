#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#define ROWS 15
#define COLS 15

char grid[ROWS][COLS];
int snake_head_x = ROWS / 2, snake_head_y = COLS / 2;
int bait_x, bait_y;
int snake_length = 1;
int *snake_tail_x, *snake_tail_y;

struct termios orig_termios;

// Function to reset terminal on exit
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    system("clear");
}

// Signal handler for graceful exit
void handle_signal(int sig) {
    free(snake_tail_x);
    free(snake_tail_y);
    reset_terminal();
    printf("\nGame exited gracefully.\n");
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

// Place bait at a random location
void place_bait() {
    do {
        bait_x = rand() % ROWS;
        bait_y = rand() % COLS;
    } while (grid[bait_x][bait_y] != '.');   // avoid placing bait on the snake
    grid[bait_x][bait_y] = 'X';
}

// Initialize grid
void initialize_grid() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = '.';
        }
    }
    grid[snake_head_x][snake_head_y] = 'O';

    // Dynamically allocate memory for snake tail
    snake_tail_x = malloc(sizeof(int) * ROWS * COLS);
    snake_tail_y = malloc(sizeof(int) * ROWS * COLS);
    if (!snake_tail_x || !snake_tail_y) {
        perror("Memory allocation failed");
        exit(1);
    }

    place_bait();
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
}

// Move the snake
void move_snake(char direction) {
    int new_head_x = snake_head_x;
    int new_head_y = snake_head_y;

    // Calculate new head position based on the direction
    switch (direction) {
        case 'w': new_head_x--; break;
        case 'a': new_head_y--; break;
        case 's': new_head_x++; break;
        case 'd': new_head_y++; break;
    }

    // Check for border collision
    if (new_head_x < 0 || new_head_x >= ROWS || new_head_y < 0 || new_head_y >= COLS) {
        return; // Do not update the snake's position
    }

    // Check for self-collision
    for (int i = 0; i < snake_length; i++) {
        if (snake_tail_x[i] == new_head_x && snake_tail_y[i] == new_head_y) {
            return; // Do not update the snake's position
        }
    }

    // Check if the snake eats the bait
    if (new_head_x == bait_x && new_head_y == bait_y) {
        snake_length++; // Grow the snake
        place_bait();   // Generate a new bait
    } else {
        // Clear the last tail position
        grid[snake_tail_x[snake_length - 1]][snake_tail_y[snake_length - 1]] = '.';
    }

    
    // Shift body positions
    for (int i = snake_length - 1; i > 0; i--) {
        snake_tail_x[i] = snake_tail_x[i - 1];
        snake_tail_y[i] = snake_tail_y[i - 1];
    }

    // Update the head position
    snake_tail_x[0] = snake_head_x;
    snake_tail_y[0] = snake_head_y;

    snake_head_x = new_head_x;
    snake_head_y = new_head_y;

    // Update the grid
    grid[snake_head_x][snake_head_y] = 'O';

    for (int i = 0; i < snake_length; i++) {
        grid[snake_tail_x[i]][snake_tail_y[i]] = '#';
    }

    // grid[snake_tail_x[snake_length - 1]][snake_tail_y[snake_length - 1]] = '.';
}

// Main function
int main() {
    char direction = 'w';

    printf("%c]0;%s%c", '\033', "snake", '\007');

    srand(time(NULL));
    setup_terminal();
    initialize_grid();

    while (1) {
        display_grid();

        if (kbhit()) {
            char input = getchar();
            if (input == 'q') break;
            if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
                direction = input;
            }
        }

        move_snake(direction);
        usleep(100000); // Slow down the game loop
    }

    handle_signal(0);
    return 0;
}
