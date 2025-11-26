#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

void memdump(const void *addr, size_t size) {
    const unsigned char *p = (const unsigned char *)addr;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("0x%08X: ", (unsigned int)(uintptr_t)(p + i));

        // print hex bytes
        for (j = 0; j < 16; j++) {
            if (i + j < size) printf("%02X ", p[i + j]);
            else printf("   ");
        }

        // print ASCII representation
        printf(" |");
        for (j = 0; j < 16 && (i + j) < size; j++) {
            unsigned char c = p[i + j];
            if (c >= 32 && c <= 126) printf("%c", c);
            else printf(".");
        }
        printf("|\n");
    }
}