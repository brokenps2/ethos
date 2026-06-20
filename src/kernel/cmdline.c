#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "string.h"
#include "drivers/pci.h"
#include "drivers/ata.h"
#include "kernel/kernel.h"
#include "drivers/terminal.h"
#include "drivers/fat32/fat32.h"
#include "drivers/keyboard.h"
#include "cmdline.h"
#include "arch/i386/ports.h"
#include "kernel/utils.h"

extern size_t term_column;
extern size_t term_width;

uint8_t min_col = 0;

extern char keybuf[KEYBUF_SIZE];
extern int head, tail;
extern bool extended;
char line[128];
int len = 0;

Fat32Context fs;

void print_prompt() {
    printf("> ");
    min_col = term_column;
}

void do_help(int argc, char** argv);

void do_cd(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: cd <directory>\n");
        return;
    }
    fat32_change_dir(&fs, argv[1]);
}

void do_cat(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: cat <filename>\n");
        return;
    }

    Fat32File file;
    fat32_open(&fs, &file, argv[1]);

    uint8_t* buffer;
    buffer = (uint8_t*)kmalloc(file.size);
    if(fat32_read(&fs, &file, buffer, file.size)) {
        buffer[file.size] = '\0';
        printf("%s", (const char*)buffer);
    }
    printf("\n");
}

void do_write(int argc, char** argv) {
    if(argc < 3) {
        printf("usage: write <data> <filename>\n");
        return;
    }
    fat32_write_file(&fs, argv[2], (uint8_t*)argv[1], strlen(argv[1]));
}

void do_rm(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: rm <filename>\n");
        return;
    }
    fat32_delete_file(&fs, argv[1]);
}

void do_run(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: run <filename>\n");
        return;
    }

	load_and_run_binary(&fs, argv[1]);
    
}

void do_hexdump(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: hexdump <filename>\n");
        return;
    }

    Fat32File file;
    fat32_open(&fs, &file, argv[1]);

    uint8_t* buffer = (uint8_t*)kmalloc(file.size);

    if (fat32_read(&fs, &file, buffer, file.size)) {
        for (int i = 0; i < file.size; i++) {
            if (buffer[i] < 16) {
                printf("0");
            }
			if(term_column >= term_width - 3) {
            	printf("%x\n", buffer[i]);
			} else {
            	printf("%x ", buffer[i]);
			}
        }
    }
    printf("\n");
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
        fat32_init(&fs);
    }
}

void do_ls(int argc, char** argv) {
    fat32_list_current_dir_cluster(&fs);
}

void do_crash(int argc, char** argv) {
    asm volatile("int $0x01");
}

void do_pciscan(int argc, char** argv) {
    pci_scan();
}

Command cmdTable[] = {
    {"help", do_help},
    {"cls", term_clear},
    {"cat", do_cat},
	{"hexdump", do_hexdump},
    {"cd", do_cd},
    {"write", do_write},
    {"rm", do_rm},
    {"ls", do_ls},
    {"diskinit", do_diskinit},
    {"crash", do_crash},
    {"pciscan", do_pciscan},
    {"reboot", do_reboot},
	{"run", do_run},
    
	{NULL, NULL}
};

void do_help(int argc, char** argv) {
    for(int i = 0; cmdTable[i].name != NULL; i++) {
        printf("%s, ", cmdTable[i].name);      
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
            char c = keymap_us[scancode];
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
