// libc/string.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <cyrillic.h>

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else if (d > s) {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

void *memset(void *pointer, int value, size_t count) {
    uint8_t *p = (uint8_t*)pointer;
    uint8_t byte = (uint8_t)value;

    while (count && ((uintptr_t)p % sizeof(uintptr_t))) {
        *p++ = byte;
        count--;
    }

    uintptr_t word = 0;
    for (size_t i = 0; i < sizeof(uintptr_t); i++) {
        word = (word << 8) | byte;
    }

    uintptr_t *pw = (uintptr_t*)p;
    while (count >= sizeof(uintptr_t)) {
        *pw++ = word;
        count -= sizeof(uintptr_t);
    }
    p = (uint8_t*)pw;
    while (count--) {
        *p++ = byte;
    }

    return pointer;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i]) return a[i] - b[i];
    return 0;
}

/* ---------------- String functions ---------------- */

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char *dst, const char *src, size_t n) {
    if (!dst || n == 0) return NULL;
    if (!src) src = "";
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return dst;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    size_t i = 0;
    for (; i < n && src[i]; i++) d[i] = src[i];
    d[i] = '\0';
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    for (; i < n && s1[i] && s1[i] == s2[i]; i++);
    if (i == n) return 0;
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

char *strchr(const char *s, int c) {
    for (; *s; s++)
        if (*s == (char)c) return (char *)s;
    return c == 0 ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    for (; *s; s++)
        if (*s == (char)c) last = s;
    if (c == 0) return (char *)s;
    return (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* next;
    if (str) next = str;
    if (!next) return NULL;

    while (*next && strchr(delim, *next)) next++;
    if (!*next) {
        next = NULL;
        return NULL;
    }

    char* token_start = next;

    while (*next && !strchr(delim, *next)) next++;
    if (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    return token_start;
}

int isspacek(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

long unsigned int strtoul(const char *str, char **endptr, int base) {
    const char *s = str;
    uint32_t result = 0;
    int neg = 0;

    // skip whitespace
    while (isspacek(*s)) s++;

    // optional sign
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;

    // auto-detect base
    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            s += 2;
        } else if (s[0] == '0') {
            base = 8;
            s += 1;
        } else {
            base = 10;
        }
    } else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }

    // parse digits
    while (*s) {
        char c = *s;
        int digit;

        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;

        if (digit >= base) break;

        result = result * base + digit;
        s++;
    }

    if (endptr) *endptr = (char *)s;
    return neg ? -result : result;
}
