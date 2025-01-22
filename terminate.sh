DISK_IMAGE="storage_vgc.img"
MOUNT_DIR="mount"
DEVICE_FILE="device-file"

# Unmount the mount directory if mounted
if mountpoint -q $MOUNT_DIR; then
    echo "Unmounting $MOUNT_DIR..."
    sudo umount $MOUNT_DIR
else
    echo "$MOUNT_DIR is not mounted. Skipping unmount."
fi

# also remove the mount directory
if [ -d "$MOUNT_DIR" ]; then
    echo "Deleting the mount directory..."
    rm -rf $MOUNT_DIR
fi

# Detach any loop device associated with the disk image
LOOP_DEVICE=$(sudo losetup -j $DISK_IMAGE | cut -d':' -f1)
if [ -n "$LOOP_DEVICE" ]; then
    echo "Detaching loop device $LOOP_DEVICE..."
    sudo losetup -d $LOOP_DEVICE
else
    echo "No loop device attached to $DISK_IMAGE. Skipping detach."
fi

# Remove the symbolic link to the device file
if [ -L "$DEVICE_FILE" ]; then
    echo "Removing symbolic link $DEVICE_FILE..."
    rm -f $DEVICE_FILE
else
    echo "Symbolic link $DEVICE_FILE does not exist. Skipping removal."
fi

echo "Termination complete."