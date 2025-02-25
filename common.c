#include "common.h"

// Defines shared functionality for kernel mode and userland programs

void *memset(void *buf,  char val, size_t size) {
    uint8_t *p = (uint8_t *) buf;
    while (size) {
        *p = val;
        p++;
        size--;
    }

    return buf;
}

void *memcpy(void *dst, const void *src, size_t n) {

    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;
    while (n--) {
        *d++ = *s++;
    }

    return dst;
}


// Vulnerability: strcpy can write past dst's memory bounds
char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}



void putchar(char c);




void printf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    while(*fmt) {
        if (*fmt == '%') {
            fmt++; // skip

            switch (*fmt) {
                case '\0': // '%' at the end of string
                    putchar('%');
                    goto end;

                case '%': // Double percent, so print percent
                    putchar('%');
                    break;
                
                case 's': //null-terminated string
                    const char *s = va_arg(vargs, const char *);
                    while (*s) {
                        putchar(*s);
                        s++;
                    }
                    break;
                
                case 'd':
                    int value = va_arg(vargs, int);
                    unsigned magnitude = value;
                    if (value < 0) {
                        putchar('-');
                        magnitude = -magnitude;
                    }

                    unsigned divisor = 1;
                    while (magnitude / divisor > 9) {
                        divisor *= 10;
                    }

                    while (divisor > 0) {
                        putchar('0' + magnitude / divisor);
                        
                        magnitude %= divisor;
                        divisor /= 10;
                    }

                    break;
                
                case 'x': {
                    const char* symbols = "0123456789abcdef";
                    unsigned value = va_arg(vargs, unsigned);
                    for (int i = 7; i >= 0; i--) {
                        unsigned nibble = (value >> (i * 4)) & 0xF;
                        putchar(symbols[nibble]);
                    }
                
                }
            }


        }
        else {
            putchar(*fmt);
        }

        fmt++;
    }


    end:
        va_end(vargs);
}


