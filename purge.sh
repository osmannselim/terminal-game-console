DISK_IMAGE="storage_vgc.img"
DEVICE_FILE="device-file"

MOUNT="mount"

./terminate.sh

# remove the bin directory
if [ -d "bin" ]; then
    echo "Deleting the bin directory..."
    rm -rf bin
fi

# remove the mount directory
if [ -d "$MOUNT" ]; then
    echo "Deleting the mount directory..."
    rm -rf $MOUNT
fi

# Remove the disk image
if [ -f "$DISK_IMAGE" ]; then
    echo "Deleting the disk image $DISK_IMAGE..."
    rm -f $DISK_IMAGE
else
    echo "No disk image found."
fi

echo "Purge complete. Environment cleaned."

