# terminal-game-console
A terminal-based video game console that supports multiple games (Tic Tac Toe, Snake, Avoid Blocks) with a virtual disk management system for environment setup and cleanup. Includes robust shell scripts for initialization, startup, termination, and purging.

# file structure

├── src

   ├── avoid_blocks.c        # Avoid Blocks game source code
   
   ├── mainscreen.c          # Main menu for game selection
   
   ├── snake.c               # Snake game source code
   
   ├── tic_tac_toe.c         # Tic Tac Toe game source code
   
├── initialize.sh            # Script to create and initialize the virtual disk

├── purge.sh                 # Script to clean up all resources, including the disk image

├── startup.sh               # Script to mount and prepare the virtual disk

├── terminate.sh             # Script to unmount and detach the virtual disk


# Features
1. Games

**Avoid Blocks:**

A reflex-based game where the player dodges falling blocks using keyboard inputs.

Written in src/avoid_blocks.c.


**Snake:**


A classic snake game where the snake grows longer by eating bait while avoiding collisions.

Includes real-time movement and instant quit functionality.

Written in src/snake.c.

**Tic Tac Toe:**


A turn-based two-player game.

Includes real-time keyboard input processing using read() for instant quit with q.

Displays the final score and game result.

Written in src/tic_tac_toe.c.


**Main Menu:**

The main interface for selecting and launching games.

Supports navigation using w, s, a, and d keys.

Features an "Exit" option.

Gracefully returns to the main menu when a game is terminated.

Written in src/mainscreen.c.

**2. Shell Scripts**

**initialize.sh:**

Creates a 100MB disk image named storage_vgc.img.

Formats the disk as ext4.

Ensures the disk image is overwritten if it already exists.


**startup.sh:**


Mounts the disk image and attaches it to a loop device.

Creates a symbolic link device-file to the loop device.

Ensures the environment is ready for use.


**terminate.sh:**

Unmounts the virtual disk and detaches the loop device.

Cleans up all files created during startup.sh.


**purge.sh:**

Performs the same tasks as terminate.sh.

Additionally deletes the disk image (storage_vgc.img).

# How to Use

1. Initialize the Environment

Run initialize.sh to create and format the virtual disk:

**sudo ./initialize.sh**

2. Start the Console

Run startup.sh to mount the disk and prepare the environment:

**sudo ./startup.sh**

3. Launch the Main Menu

Compile and run mainscreen.c to start the game console:

**gcc -o main_screen src/mainscreen.c
./main_screen**

4. Select and Play a Game
Use the following keys in the main menu:

w / s: Navigate between games.

a / d: Switch between game selection and the "Exit" option.

Enter: Launch the selected game or exit the console.

Each game runs independently and gracefully returns to the main menu upon termination.

6. Terminate the Console
7. 
Run terminate.sh to unmount the disk and clean up the environment:

**sudo ./terminate.sh**

6. Purge the Environment
Run purge.sh to delete the disk image and all related resources:

**sudo ./purge.sh**

# Testing and Validation

**Graceful Termination:** 

All games exit gracefully on SIGINT

The main menu survives SIGINT and SIGTSTP signals during operation.

Script Functionality:

startup.sh works after terminate.sh without requiring initialize.sh.

purge.sh ensures complete cleanup.

# Game Behavior:

Games handle invalid inputs gracefully.

Pressing q in games terminates them immediately and returns to the main menu.

Real-time input handling ensures responsive gameplay.


# Requirements

Compiler: GCC or any standard C compiler.

Shell: Bash shell for executing the scripts.

Permissions: Root privileges for mounting, unmounting, and handling loop devices.

# Known Limitations

Games are terminal-based and require resizing if played on smaller terminals.

Disk image is limited to 100MB; adjustments may be required for larger applications.


Enjoy playing! 🎮

