#include <stdint.h>
#include <stddef.h>
#pragma once

typedef void (*CommandFunction)(int argc, char** argv);

typedef struct {
    const char* name;
    CommandFunction func;
} Command;

void print_prompt();
void handle_command(char* input);
void scan_kernel_cmdline();
