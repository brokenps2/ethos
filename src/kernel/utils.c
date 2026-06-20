#include <stdint.h>
#include "stdio.h"
#include "drivers/fat32/fat32.h"

void sleep_busy(uint32_t ticks) {
	for(uint32_t i = 0; i < ticks; i++) {
		asm volatile("nop");
	}
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str, i);
    return str;
}

void load_and_run_binary(Fat32Context* fs, const char* filename) {
	Fat32File file;

	if(!fat32_open(fs, &file, filename)) {
		printf("failed to open program file\n");
		return;
	}

	uint8_t* address = (uint8_t*)0x500000;

	fat32_read(fs, &file, address, file.size);

	void (*program_entry)() = (void(*)())address;

	program_entry();
}
