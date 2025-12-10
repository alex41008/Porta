#!/bin/bash

# Abbruch bei Fehler
set -e

# --- 1. Abhängigkeiten installieren und Verzeichnisse erstellen ---
sudo apt-get install -y g++ nasm qemu-system-x86 grub-pc-bin xorriso mtools

mkdir -p build
mkdir -p user_files

# --- 2. Disk Image erstellen und MBR/Partitionstabelle schreiben ---
DISK_IMG="build/disk.img"
DISK_SIZE="10M"

if [ ! -f "$DISK_IMG" ]; then
    echo "Creating virtual disk image (10MB)..."
    dd if=/dev/zero of="$DISK_IMG" bs=1M count=10
fi

# Partition Entry (Typ 0x83, Start LBA 1)
PARTITION_ENTRY="\x80\x00\x00\x00\x83\x00\x00\x00\x01\x00\x00\x00\xFF\x0F\x00\x00"

# Schreibe Partitionseintrag bei Offset 0x1BE
echo -en "$PARTITION_ENTRY" | dd of="$DISK_IMG" bs=1 seek=$((0x1BE)) count=16 conv=notrunc

# Schreibe MBR-Signatur bei Offset 510
echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=510 count=2 conv=notrunc

# !!! ALLE MANUELLEN VBR-DD-BEFEHLE HIER ENTFERNT (0x02, 0x2000, 0x0400, etc.) !!!

# --- 3. FAT32 Dateisystem formatieren (Korrigierte Methode) ---

# Wir müssen mkfs.fat anweisen, die Partition bei Offset 512 zu formatieren.
# Die Partitionsgröße beträgt 10MB - 1MB (für MBR/VBR/FAT). Ca. 9MB = 9 * 1024 * 1024 Bytes.
# Wir verwenden truncate, um eine temporäre Datei mit der Partitionsgröße zu erstellen.

PARTITION_START_SEEK=$((512)) # Partitionsstart bei Sektor 1

# Das Volume ist 10MB (20480 Sektoren) - 1 Sektor (MBR) = 20479 Sektoren
PARTITION_SECTORS=$((10 * 1024 * 1024 / 512 - 1)) 

echo "Formatting partition 1 at offset 512..."

# Verwende truncate/dd, um den Bereich ab Offset 512 zu formatieren (kompliziert, aber präzise)
# Da mkfs.fat keine direkten Offset-Parameter hat, muss dies indirekt erfolgen:
# 1. Erstelle eine temporäre Datei mit der Größe der Partition.
# 2. Formatiere die temporäre Datei.
# 3. Schreibe die formatierte temporäre Datei zurück in das Disk-Image bei Offset 512.

TEMP_IMG="build/part.img"
PARTITION_SIZE_BYTES=$((PARTITION_SECTORS * 512))

echo "  -> Creating temporary partition image ($PARTITION_SIZE_BYTES Bytes)..."
dd if=/dev/zero of="$TEMP_IMG" bs=1 count=0 seek=$PARTITION_SIZE_BYTES

echo "  -> Formatting temporary image..."
sudo /sbin/mkfs.fat -F 32 -n "SPECTOS" "$TEMP_IMG"

echo "  -> Writing formatted VBR/FAT/Data back to disk.img at offset $PARTITION_START_SEEK..."
# Schreibe die formatierte Partition in das Haupt-Image, beginnend bei Sektor 1.
dd if="$TEMP_IMG" of="$DISK_IMG" bs=1 seek=$PARTITION_START_SEEK conv=notrunc

rm "$TEMP_IMG"

# --- 4. Benutzerdateien in das Image kopieren (Mount-Logik) ---
echo "Copying user files from 'user_files/' to disk image via loop device..."

mkdir -p user_files

if [ -z "$(ls -A user_files 2>/dev/null)" ]; then
    echo "Dies ist eine Beispieldatei. Sie können diese ersetzen oder löschen." > user_files/SAMPLE.TXT
fi

MOUNT_POINT="build/mnt"
mkdir -p "$MOUNT_POINT"

echo "  -> Mounting partition 1 (Offset 512) for file copy..."

# Mounten mit dem korrekten Offset des Dateisystems (Partition 1 beginnt bei Sektor 1 = 512 Bytes)
sudo mount -o loop,offset=512 "$DISK_IMG" "$MOUNT_POINT" || {
    echo "Error: Mounting failed. Check if FAT is correctly placed at offset 512."
    sudo umount "$MOUNT_POINT" 2>/dev/null || true
    exit 1
}

# Kopiere alle Dateien in das gemountete Verzeichnis
echo "  -> Copying files..."
sudo cp -r user_files/* "$MOUNT_POINT/"

# Unmount und Bereinigung
echo "  -> Unmounting and cleaning up..."
sudo umount "$MOUNT_POINT"

# --- 5. Kernel kompilieren und linken ---
echo "Compiling boot.asm..."
nasm -f elf32 boot.asm -o build/boot.o

# ... (Rest des Kompilierungs- und QEMU-Codes bleibt unverändert) ...

# Compile the kernel
echo "Compiling kernel.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -O2 -c kernel.cpp -o build/kernel.o

# Link the kernel
echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o build/kernel.bin build/boot.o build/kernel.o

# Check if kernel.bin exists and has size greater than 0
if [ ! -s build/kernel.bin ]; then
    echo "Error: kernel.bin is empty or does not exist."
    exit 1
fi

# Create a bootable ISO
echo "Creating bootable ISO..."
mkdir -p build/iso/boot/grub
cp build/kernel.bin build/iso/boot/
cat > build/iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "Spectrum" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o build/cos.iso build/iso
echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=510 count=2 conv=notrunc

echo -en '\x55\xAA' | dd of="$DISK_IMG" bs=1 seek=$((512 + 510)) count=2 conv=notrunc
# Run the OS in QEMU
echo "Starting QEMU with IDE drive..."
qemu-system-i386 \
    -boot d \
    -cdrom build/cos.iso \
    -drive file="$DISK_IMG",format=raw,index=0,media=disk,if=ide \
    -d guest_errors