#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "string.h"
#include "drivers/pci.h"
#include "drivers/ata.h"
#include "kernel/kernel.h"
#include "drivers/terminal.h"
#include "drivers/fat32.h"
#include "drivers/keyboard.h"
#include "arch/i386/ports.h"

extern size_t termColumn;


uint8_t buf[512];
uint8_t min_col = 0;

extern char keybuf[KEYBUF_SIZE];
extern int head, tail;
extern char line[128];
extern int len;

void print_prompt() {
    printf("> ");
    min_col = termColumn;
}

void do_help(int argc, char** argv);

void do_cd(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: cd <directory>\n");
        return;
    }
    fat32_change_dir(argv[1]);
}

void do_cat(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: cat <filename>\n");
        return;
    }
    int size;
    size = fat32_get_file_size(argv[1]);
    if(size == -1) {
        return;
    }
    uint8_t* buffer;
    buffer = (uint8_t*)kmalloc(size);
    if(fat32_read_file(argv[1], buffer, (uint32_t*)&size)) {
        buffer[size] = '\0';
        printf("%s\n", (const char*)buffer);
    }
    
}

void do_write(int argc, char** argv) {
    if(argc < 3) {
        printf("usage: write <data> <filename>\n");
        return;
    }
    fat32_write_file(argv[2], (uint8_t*)argv[1], strlen(argv[1]));
}

void do_rm(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: rm <filename>\n");
        return;
    }
    fat32_delete_file(argv[1]);
}

void do_run(int argc, char** argv) {

    if(argc < 2) {
        printf("usage: run <filename>\n");
        return;
    }
    
}

void do_reboot() {
    while (1) {
        uint8_t status;
        asm volatile ("inb %1, %0" : "=a"(status) : "Nd"(0x64));
        if (!(status & 0x02)) break;
    }

    outb(0x64, 0xFE);

    asm volatile ("hlt");
}

void do_diskinit(int argc, char** argv) {
    if(ata_detect() != 0) {
        fat32_init();
    }
}

void do_crash(int argc, char** argv) {
    asm volatile("int $0x01");
}

void do_about(int argc, char** argv) {
    printf("ethos -- eli thomas' hobby OS -- (c)2025-2026\n");
}

void do_pciscan(int argc, char** argv) {
    pci_scan();
}

Command cmdTable[] = {
    {"help", "show cmd list", do_help},
    {"cls", "clear screen", term_clear},
    {"cat", "read file to terminal", do_cat},
    {"cd", "change dir", do_cd},
    {"write", "write data to a file", do_write},
    {"rm", "delete file", do_rm},
    {"ls", "list current dir", fat32_list_dir},
    {"dinit", "init ata & fat32", do_diskinit},
    {"crash", "throw cpu debug exception", do_crash},
    {"pci-scan", "scan pci devices", do_pciscan},
    {"about", "about ethos", do_about},
    {"reboot", "reboot", do_reboot},

    {NULL, NULL, NULL}
};

void do_help(int argc, char** argv) {
    for(int i = 0; cmdTable[i].name != NULL; i++) {
        printf("%s", cmdTable[i].name);
        for(int j = 0; j < (12 - strlen(cmdTable[i].name)); j++) {
            printf(" ");
        }
        printf("%s\n", cmdTable[i].help);
    }
    printf("\n");
}

void handle_command(char* input) {
    char* argv[16];
    int argc = 0;

    char* token = strtok(input, " ");
    while (token != NULL && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    for (int i = 0; cmdTable[i].name != NULL; i++) {
        if (strcmp(argv[0], cmdTable[i].name) == 0) {
            cmdTable[i].func(argc, argv);
            print_prompt();
            return;
        }
    }

    printf("Unknown command: %s\n", argv[0]);
    print_prompt();
}

void scan_kernel_cmdline() {
    while (tail != head) {
        unsigned char scancode = keybuf[tail];
        tail = (tail + 1) % KEYBUF_SIZE;

        if (!(scancode & 0x80)) {
            char c = keymapUS[scancode];
            if (c == '\n') {
                line[len] = 0;
                len = 0;
                term_put_char('\n');
                handle_command(line);
	        } else if(c == '\b') {
                if(len > 0) {
                    len--;
                    term_put_char('\b');
                }
            } else {
                line[len++] = c;
                term_put_char(c);
            }
        }
    }
}
