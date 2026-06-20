#include <stdint.h>
#include <stddef.h>
#pragma once

typedef void (*command_function_t)(int argc, char** argv);

typedef struct {
    const char* name;
    command_function_t func;
} command_t;

void print_prompt();
void handle_command(char* input);
void scan_kernel_cmdline();
