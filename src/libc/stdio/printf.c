#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "stdio.h"

static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++)
        if (putchar(bytes[i]) == EOF)
            return false;
    return true;
}

static bool print_int(int value) {
    char buf[12];
    int i = 0;

    if (value == 0) {
        char c = '0';
        return print(&c, 1);
    }

    bool negative = value < 0;
    if (negative)
        value = -value;

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (negative)
        buf[i++] = '-';

    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }

    return print(buf, i);
}


static bool print_hex(unsigned int value) {
    char buf[9];
    char *hex_chars = "0123456789ABCDEF";
    int i = 0;

    if (value == 0) {
        char c = '0';
        return print(&c, 1);
    }

    while (value > 0) {
        buf[i++] = hex_chars[value % 16];
        value /= 16;
    }

    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }

    return print(buf, i);
}

int printf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0') {
        size_t maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char* format_begun_at = format++;

        if (*format == 'c') {
            format++;
            char c = (char) va_arg(parameters, int /* char promotes to int */);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        } else if (*format == 's') {
            format++;
            const char* str = va_arg(parameters, const char*);
            size_t len = strlen(str);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        } else if (*format == 'd' || *format == 'i') {
            format++;
            int val = va_arg(parameters, int);
            if (!print_int(val))
                return -1;
        } else if(*format == 'x') { //doesnt work
            format++;
            //print("0x", 2);
            int val = va_arg(parameters, int);
            if(!print_hex(val))
                return -1;
        } else {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, len))
                return -1;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return written;
}

