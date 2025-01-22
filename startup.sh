# Variables
DISK_IMAGE="storage_vgc.img"
MOUNT_DIR="mount"
DEVICE_FILE="device-file"

# Check if the mount directory exists
if [ ! -d "$MOUNT_DIR" ]; then
    echo "Mount directory does not exist. Creating it..."
    mkdir -p $MOUNT_DIR
fi

# Detach any existing loop devices associated with the disk image
EXISTING_LOOP_DEVICES=$(sudo losetup -j $DISK_IMAGE | cut -d':' -f1)
if [ -n "$EXISTING_LOOP_DEVICES" ]; then
    echo "Detaching existing loop devices..."
    for LOOP in $EXISTING_LOOP_DEVICES; do
        echo "Detaching $LOOP..."
        sudo losetup -d $LOOP
    done
fi

# Attach the disk image to a new loop device
LOOP_DEVICE=$(sudo losetup -f) # Find the next available loop device
echo "Attaching $DISK_IMAGE to $LOOP_DEVICE..."
sudo losetup $LOOP_DEVICE $DISK_IMAGE

# Mount the loop device
if mountpoint -q $MOUNT_DIR; then
    echo "The mount directory is already mounted."
else
    echo "Mounting $LOOP_DEVICE to $MOUNT_DIR..."
    sudo mount $LOOP_DEVICE $MOUNT_DIR
fi

sudo gcc -o mount/game_snake src/snake.c
sudo gcc -o mount/game_tic_tac_toe src/tic_tac_toe.c
sudo gcc -o mount/game_avoid_blocks src/avoid_blocks.c
sudo gcc -o mount/main_screen src/main_screen.c

# add the executables to the mount directory


# Create a symbolic link to the device file
ln -sf $LOOP_DEVICE $DEVICE_FILE

echo "Startup complete. Virtual disk is ready for use."