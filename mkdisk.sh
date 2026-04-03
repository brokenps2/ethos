#!/bin/bash
set -e

cd "$(dirname "$0")"

IMG="./bin/disk.img"

sudo umount /mnt 2>/dev/null || true
sudo losetup -D

dd if=/dev/zero of="$IMG" bs=512 count=131072

printf "2048,,L,*\n" | sudo sfdisk "$IMG"

LOOP=$(sudo losetup --find --show "$IMG")
PART=$(sudo losetup --find --show -o 1048576 "$IMG")

sudo mkfs.fat -F32 "$PART"

sudo mkdir -p /mnt
sudo mount "$PART" /mnt

sudo grub-install \
  --target=i386-pc \
  --boot-directory=/mnt/boot \
  --modules="normal part_msdos fat multiboot biosdisk" \
  "$LOOP"

sudo cp ./bin/ethos.bin /mnt/boot/
sudo mkdir -p /mnt/boot/grub
sudo cp ./bin/grub.cfg /mnt/boot/grub/

sync
