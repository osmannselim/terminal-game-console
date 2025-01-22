#!/bin/bash

# create the first running environment since we will submit this without /bin file
# create the bin directory
sudo mkdir -p bin
sudo gcc -o bin/game_snake src/snake.c
sudo gcc -o bin/game_tic_tac_toe src/tic_tac_toe.c
sudo gcc -o bin/game_avoid_blocks src/avoid_blocks.c
sudo gcc -o bin/main_screen src/main_screen.c


# Name of the virtual disk image
DISK_IMAGE="storage_vgc.img"

# Directories
BIN_DIR="bin"
SRC_DIR="src"

# Check if the image already exists
if [ -f "$DISK_IMAGE" ]; then
    echo "Image already exists. Removing the old image..."
    sudo rm -f "$DISK_IMAGE"
fi

# Create a new disk image (100MB)
echo "Creating a new disk image: $DISK_IMAGE..."
dd if=/dev/zero of=$DISK_IMAGE bs=1M count=100

# Format the disk image as ext4
echo "Formatting the disk image as ext4..."
sudo mkfs.ext4 "$DISK_IMAGE"

echo "Initialization complete. You can now use the disk image."