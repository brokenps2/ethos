#!/bin/bash
set -e

cd "$(dirname "$0")"

IMG="./bin/disk.img"

MNTPNT="/mnt/ethosdisk"

sudo umount $MNTPNT 2>/dev/null || true
sudo losetup -D

dd if=/dev/zero of="$IMG" bs=512 count=131072

printf "2048,,L,*\n" | sudo sfdisk "$IMG"

LOOP=$(sudo losetup --find --show "$IMG")
PART=$(sudo losetup --find --show -o 1048576 "$IMG")

sudo mkfs.fat -F32 "$PART"

sudo mkdir -p $MNTPNT
sudo mount "$PART" $MNTPNT

sudo grub-install \
  --target=i386-pc \
  --boot-directory=$MNTPNT/boot \
  --modules="normal part_msdos fat multiboot biosdisk" \
  "$LOOP"

sudo cp ./bin/ethos.bin $MNTPNT/boot/
sudo mkdir -p $MNTPNT/boot/grub
sudo cp ./bin/grub.cfg $MNTPNT/boot/grub/

sync
