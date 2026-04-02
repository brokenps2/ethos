qemu-system-i386 -s -S --kernel etphos.bin -drive file=disk.img,format=raw,index=0,media=disk -boot order=d &
gdb etphos.bin
