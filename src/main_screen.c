#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_GAMES 10

// Global variables
struct termios orig_termios;
char *games[MAX_GAMES];
int game_count = 0;
int selected_game = 0; // Tracks the selected game
int is_exit_selected = 0; // Tracks whether the Exit button is selected
pid_t child_pid = -1;    // Track the child process ID

// Function Prototypes
void reset_terminal();
void setup_terminal();
void ensure_terminal_reset();
void handle_signal(int sig);
void display_main_screen();
void launch_game(const char *game);
int kbhit();
void scan_games();

// Reset terminal to its original settings
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore original terminal settings
    system("clear");
    printf("\033[0m"); // Reset text formatting
}

// Ensure terminal is reset on exit
void ensure_terminal_reset() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); 
    reset_terminal();
}

void handle_signal(int sig) {
    if (child_pid > 0) {
        // Child process (game) is running, terminate it
        printf("\nGame interrupted. Returning to main menu...\n");
        kill(child_pid, SIGKILL);  // Force terminate the child process
        waitpid(child_pid, NULL, 0); // Wait for the child process to terminate
        child_pid = -1;  // Reset the child process ID
        display_main_screen();  // Return to the main menu
    } else {
        // Parent process: exit gracefully if no child is running
        if (sig == SIGINT || sig == SIGTERM) {
            printf("\nExiting main menu gracefully...\n");
            ensure_terminal_reset();
            exit(0);
        }
    }
}

// Setup terminal for non-canonical input
void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // Get original terminal settings
    atexit(ensure_terminal_reset);         // Ensure terminal reset on exit
    signal(SIGINT, handle_signal);         // Handle "Ctrl+C"
    signal(SIGTERM, handle_signal);        // Handle termination

    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echoing
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

// Scan the current directory for executable game files starting with "game_"
void scan_games() {
    struct dirent *entry;
    DIR *dp = opendir(".");

    if (!dp) {
        perror("Unable to open directory");
        exit(1);
    }

    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "game_", 5) == 0 && game_count < MAX_GAMES) {
            games[game_count++] = strdup(entry->d_name);
        }
    }
    closedir(dp);

    if (game_count == 0) {
        printf("No games found. Ensure game executables are in the current directory.\n");
        exit(1);
    }
}

// Display the main menu
void display_main_screen() {
    system("clear");
    printf("\n=== Welcome to main-screen ===\n\n");

    int padding = 20 - strlen(games[selected_game]);

    if (is_exit_selected) {
        // get only the game name without the "game_" prefix
        printf("  Game:   %s      ", games[selected_game] + 5);
        for (int i = 0 ; i< padding; i++){
            printf(" ");
        }
        printf("  > Exit <\n");
    } else {
        printf("  Game: > %s <      ", games[selected_game] + 5);
        for (int i = 0 ; i< padding; i++){
            printf(" ");
        }
        printf("  Exit\n");
    }

    printf("\nUse 'w' and 's' to change game.\n");
    printf("Use 'a' and 'd' to select Exit.\n");
    printf("Press 'Enter' to confirm.\n");
}

// Launch the selected game using fork and exec
void launch_game(const char *game) {
    child_pid = fork();

    if (child_pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (child_pid == 0) {
        // Child process: launch the game
        reset_terminal();          // Reset terminal in child process
        char game_path[256];
        snprintf(game_path, sizeof(game_path), "./%s", game); // Build path to the game
        execlp(game_path, game, NULL); // Execute the game
        perror("Failed to launch game"); // Handle execlp failure
        exit(1); // Exit the child process
    } else {
        // Parent process: wait for the child process to finish
        int status;
        waitpid(child_pid, &status, 0); // Wait for the game process to complete
        // now we need to update the title of the console
        printf("%c]0;%s%c", '\033', "main-screen", '\007');
        child_pid = -1; // Reset the child process ID
        setup_terminal(); // Reconfigure the terminal for the main menu
        display_main_screen(); // Re-display the main menu
    }
}

// Main function
int main(int argc, char *argv[]) {
    printf("%c]0;%s%c", '\033', "main-screen", '\007');
    setup_terminal();
    scan_games();
    display_main_screen();

    while (1) {
        if (kbhit()) {
            char input = getchar();

            switch (input) {
                case 'q': // Quit the main menu
                    handle_signal(SIGTERM);
                    break;
                case 'w': // Move up in the game list
                    if (!is_exit_selected) {
                        selected_game = (selected_game - 1 + game_count) % game_count;
                        display_main_screen();
                    }
                    break;
                case 's': // Move down in the game list
                    if (!is_exit_selected) {
                        selected_game = (selected_game + 1) % game_count;
                        display_main_screen();
                    }
                    break;
                case 'a': // Select Exit
                    is_exit_selected = !is_exit_selected;
                    display_main_screen();
                    break;
                case 'd': // Deselect Exit
                    is_exit_selected = !is_exit_selected;
                    display_main_screen();
                    break;
                case '\n': // Enter key to select an option
                    if (is_exit_selected) {
                        handle_signal(SIGTERM); // Exit the main menu
                    } else {
                        launch_game(games[selected_game]);
                    }
                    break;
            }
        }

        usleep(100000); // Smooth game loop
    }

    return 0;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <signal.h>
// #include <termios.h>
// #include <unistd.h>
// #include <sys/select.h>
// #include <time.h>
// #include <sys/wait.h>

// #define MAX_GAMES 10

// // Global variables
// struct termios orig_termios;
// char *games[MAX_GAMES];
// int game_count = 0;
// int selected_game = 0; // Tracks the selected game
// int is_exit_selected = 0; // Tracks whether the Exit button is selected
// pid_t child_pid = -1;    // Track the child process ID

// // Function Prototypes
// void reset_terminal();
// void setup_terminal();
// void ensure_terminal_reset();
// void handle_signal(int sig);
// void display_main_screen();
// void launch_game(const char *game);
// int kbhit();
// void scan_games();

// // Reset terminal to its original settings
// void reset_terminal() {
//     tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore original terminal settings
//     system("clear");
//     printf("\033[0m"); // Reset text formatting
// }

// // Ensure terminal is reset on exit
// void ensure_terminal_reset() {
//     tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); 
//     reset_terminal();
// }
// void handle_signal(int sig) {
//     if (child_pid > 0) {
//         // Child process (game) is running, terminate it
//         printf("\nGame interrupted. Returning to main menu...\n");
//         kill(child_pid, SIGKILL);  // Force terminate the child process
//         waitpid(child_pid, NULL, 0); // Wait for the child process to terminate
//         child_pid = -1;  // Reset the child process ID
//         display_main_screen();  // Return to the main menu
//     } else {
//         // Parent process: exit gracefully if no child is running
//         if (sig == SIGINT || sig == SIGTERM) {
//             printf("\nExiting main menu gracefully...\n");
//             ensure_terminal_reset();
//             exit(0);
//         }
//     }
// }

// // Setup terminal for non-canonical input
// void setup_terminal() {
//     struct termios new_termios;
//     tcgetattr(STDIN_FILENO, &orig_termios); // Get original terminal settings
//     atexit(ensure_terminal_reset);         // Ensure terminal reset on exit
//     signal(SIGINT, handle_signal);         // Handle "Ctrl+C"
//     signal(SIGTERM, handle_signal);        // Handle termination

//     new_termios = orig_termios;
//     new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echoing
//     tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
// }

// // Non-blocking input
// int kbhit() {
//     struct timeval tv = {0, 0};
//     fd_set fds;
//     FD_ZERO(&fds);
//     FD_SET(STDIN_FILENO, &fds);
//     select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
//     return FD_ISSET(STDIN_FILENO, &fds);
// }

// // Scan the current directory for executable game files starting with "game_"
// void scan_games() {
//     struct dirent *entry;
//     DIR *dp = opendir(".");

//     if (!dp) {
//         perror("Unable to open directory");
//         exit(1);
//     }

//     while ((entry = readdir(dp))) {
//         if (strncmp(entry->d_name, "game_", 5) == 0 && game_count < MAX_GAMES) {
//             games[game_count++] = strdup(entry->d_name);
//         }
//     }
//     closedir(dp);

//     if (game_count == 0) {
//         printf("No games found. Ensure game executables are in the current directory.\n");
//         exit(1);
//     }
// }

// // Display the main menu
// void display_main_screen() {
//     system("clear");
//     printf("\n=== Welcome to Ottoman Games! ===\n\n");

//     int padding = 20 - strlen(games[selected_game]);

//     if (is_exit_selected) {
//         // get only the game name without the "game_" prefix
//         printf("  Game:   %s      ", games[selected_game] + 5);
//         for (int i = 0 ; i< padding; i++){
//             printf(" ");
//         }
//         printf("  > Exit <\n");
//     } else {
//         printf("  Game: > %s <      ", games[selected_game] + 5);
//         for (int i = 0 ; i< padding; i++){
//             printf(" ");
//         }
//         printf("  Exit\n");
//     }

//     printf("\nUse 'w' and 's' to change game.\n");
//     printf("Use 'a' and 'd' to select Exit.\n");
//     printf("Press 'Enter' to confirm.\n");
// }

// // Launch the selected game using fork and exec
// void launch_game(const char *game) {
//     child_pid = fork();

//     if (child_pid < 0) {
//         perror("Fork failed");
//         exit(1);
//     } else if (child_pid == 0) {
//         // Child process: launch the game
//         reset_terminal();          // Reset terminal in child process
//         char game_path[256];
//         snprintf(game_path, sizeof(game_path), "./%s", game); // Build path to the game
//         execlp(game_path, game, NULL); // Execute the game
//         perror("Failed to launch game"); // Handle execlp failure
//         exit(1); // Exit the child process
//     } else {
//         // Parent process: wait for the child process to finish
//         int status;
//         waitpid(child_pid, &status, 0); // Wait for the game process to complete
//         child_pid = -1; // Reset the child process ID
//         setup_terminal(); // Reconfigure the terminal for the main menu
//         display_main_screen(); // Re-display the main menu
//     }
// }



// // Main function
// int main() {
//     setup_terminal();
//     scan_games();
//     display_main_screen();

//     while (1) {
//         if (kbhit()) {
//             char input = getchar();

//             switch (input) {
//                 case 'q': // Quit the main menu
//                     handle_signal(SIGTERM);
//                     break;
//                 case 'w': // Move up in the game list
//                     if (!is_exit_selected) {
//                         selected_game = (selected_game - 1 + game_count) % game_count;
//                         display_main_screen();
//                     }
//                     break;
//                 case 's': // Move down in the game list
//                     if (!is_exit_selected) {
//                         selected_game = (selected_game + 1) % game_count;
//                         display_main_screen();
//                     }
//                     break;
//                 case 'a': // Select Exit
//                     //  take the inverse of is_exit_selected, 0 ---> 1, 1 ---> 0
//                     is_exit_selected = !is_exit_selected;
//                     // is_exit_selected = 0;
//                     display_main_screen();
//                     break;
//                 case 'd': // Deselect Exit
//                     //  take the inverse of is_exit_selected, 0 ---> 1, 1 ---> 0
//                     is_exit_selected = !is_exit_selected;
//                     // is_exit_selected = 0;
//                     display_main_screen();
//                     break;
//                 case '\n': // Enter key to select an option
//                     if (is_exit_selected) {
//                         handle_signal(SIGTERM); // Exit the main menu
//                     } else {
//                         launch_game(games[selected_game]);
//                     }
//                     break;
//             }
//         }

//         usleep(100000); // Smooth game loop
//     }

//     return 0;
// }
