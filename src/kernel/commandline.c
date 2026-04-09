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
    {"clear", "clear screen", term_clear},
    {"cat", "read file to terminal", do_cat},
    {"cd", "change dir", do_cd},
    {"ls", "list current dir", fat32_list_dir},
    {"disk-init", "init disk system", do_diskinit},
    {"crash", "throw debug exception", do_crash},
    {"pci-scan", "scan for pci devices", do_pciscan},

    {NULL, NULL, NULL}
};

void do_help(int argc, char** argv) {
    for(int i = 0; cmdTable[i].name != NULL; i++) {
        printf("%s ", cmdTable[i].name);
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
