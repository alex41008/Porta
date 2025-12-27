#!/bin/bash

set -e

sudo apt-get install -y g++ nasm qemu-system-x86 grub-pc-bin xorriso mtools

mkdir -p build
mkdir -p user_files

DISK_IMG="build/disk.img"
DISK_SIZE="10M"

if [ ! -f "$DISK_IMG" ]; then
    echo "Creating virtual disk image (10MB)..."
    dd if=/dev/zero of="$DISK_IMG" bs=1M count=10
fi

PARTITION_ENTRY="\x80\x00\x00\x00\x83\x00\x00\x00\x01\x00\x00\x00\xFF\x0F\x00\x00"

echo -en "$PARTITION_ENTRY" | dd of="$DISK_IMG" bs=1 seek=$((0x1BE)) count=16 conv=notrunc

echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=510 count=2 conv=notrunc

PARTITION_START_SEEK=$((512))

PARTITION_SECTORS=$((10 * 1024 * 1024 / 512 - 1)) 

echo "Formatting partition 1 at offset 512..."


TEMP_IMG="build/part.img"
PARTITION_SIZE_BYTES=$((PARTITION_SECTORS * 512))

echo "  -> Creating temporary partition image ($PARTITION_SIZE_BYTES Bytes)..."
dd if=/dev/zero of="$TEMP_IMG" bs=1 count=0 seek=$PARTITION_SIZE_BYTES

echo "  -> Formatting temporary image..."
sudo /sbin/mkfs.fat -F 32 -n "PORTAOS" "$TEMP_IMG"

echo "  -> Writing formatted VBR/FAT/Data back to disk.img at offset $PARTITION_START_SEEK..."

dd if="$TEMP_IMG" of="$DISK_IMG" bs=1 seek=$PARTITION_START_SEEK conv=notrunc

rm "$TEMP_IMG"

echo "Copying user files from 'user_files/' to disk image via loop device..."

mkdir -p user_files

MOUNT_POINT="build/mnt"
mkdir -p "$MOUNT_POINT"

echo "  -> Mounting partition 1 (Offset 512) for file copy..."

sudo mount -o loop,offset=512 "$DISK_IMG" "$MOUNT_POINT" || {
    echo "Error: Mounting failed. Check if FAT is correctly placed at offset 512."
    sudo umount "$MOUNT_POINT" 2>/dev/null || true
    exit 1
}

echo "  -> Copying files..."
sudo cp -r user_files/* "$MOUNT_POINT/"

echo "  -> Unmounting and cleaning up..."
sudo umount "$MOUNT_POINT"

echo "Compiling boot.asm..."
nasm -f elf32 boot.asm -o build/boot.o

echo "Compiling sys_asm_funcs.asm..."
nasm -f elf32 sys_asm_funcs.asm -o build/sys_asm_funcs.o

echo "Compiling sys_input.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_input.cpp -o build/sys_input.o

echo "Compiling sys_cli_task.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_cli_task.cpp -o build/sys_cli_task.o

echo "Compiling sys_process.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemprocess/sys_process.cpp -o build/sys_process.o

echo "Compiling sys_clock_task.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_clock_task.cpp -o build/sys_clock_task.o

echo "Compiling sys_scheduler.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemprocess/sys_scheduler.cpp -o build/sys_scheduler.o

echo "Compiling sys_window.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemprocess/sys_window.cpp -o build/sys_window.o

echo "Compiling sys_font.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_font.cpp -o build/sys_font.o

echo "Compiling sys_ps2_mouse_driver.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_ps2_mouse_driver.cpp -o build/sys_ps2_mouse_driver.o

echo "Compiling sys_vga13h_screen.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c systemimpl/sys_vga13h_screen.cpp -o build/sys_vga13h_screen.o

echo "Compiling kernel.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c kernel.cpp -o build/kernel.o

echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o build/kernel.bin build/boot.o build/kernel.o build/sys_asm_funcs.o build/sys_input.o build/sys_cli_task.o build/sys_process.o build/sys_clock_task.o build/sys_scheduler.o build/sys_font.o build/sys_window.o build/sys_ps2_mouse_driver.o build/sys_vga13h_screen.o

if [ ! -s build/kernel.bin ]; then
    echo "Error: kernel.bin is empty or does not exist."
    exit 1
fi

echo "Creating bootable ISO..."
mkdir -p build/iso/boot/grub
cp build/kernel.bin build/iso/boot/
cat > build/iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "Porta" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o build/cos.iso build/iso
echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=510 count=2 conv=notrunc

echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=$((512 + 510)) count=2 conv=notrunc

echo "Starting QEMU with IDE drive..."
qemu-system-i386 \
    -boot d \
    -cdrom build/cos.iso \
    -drive file="$DISK_IMG",format=raw,index=0,media=disk,if=ide \
    -d guest_errors