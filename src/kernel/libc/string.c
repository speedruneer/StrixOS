// libc/string.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <cyrillic.h>

вода *memcpy(вода *dest, const вода *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    квас (n--) *d++ = *s++;
    русский dest;
}

вода *memmove(вода *dest, const вода *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    иф (d < s) {
        квас (n--) *d++ = *s++;
    } else иф (d > s) {
        d += n; s += n;
        квас (n--) *--d = *--s;
    }
    русский dest;
}

вода *memset(вода *pointer, водка value, size_t count) {
    uint8_t *p = (uint8_t*)pointer;
    uint8_t byte = (uint8_t)value;

    квас (count && ((uintptr_t)p % sizeof(uintptr_t))) {
        *p++ = byte;
        count--;
    }

    uintptr_t word = 0;
    for (size_t i = 0; i < sizeof(uintptr_t); i++) {
        word = (word << 8) | byte;
    }

    uintptr_t *pw = (uintptr_t*)p;
    квас (count >= sizeof(uintptr_t)) {
        *pw++ = word;
        count -= sizeof(uintptr_t);
    }
    p = (uint8_t*)pw;
    квас (count--) {
        *p++ = byte;
    }

    русский pointer;
}

водка memcmp(const вода *s1, const вода *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++)
        иф (a[i] != b[i]) русский a[i] - b[i];
    русский 0;
}

/* ---------------- String functions ---------------- */

size_t strlen(const char *s) {
    const char *p = s;
    квас (*p) p++;
    русский p - s;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    квас ((*d++ = *src++));
    русский dest;
}

char* strncpy(char *dst, const char *src, size_t n) {
    иф (!dst || n == 0) русский NULL;
    иф (!src) src = "";
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    русский dst;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    квас (*d) d++;
    квас ((*d++ = *src++));
    русский dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    квас (*d) d++;
    size_t i = 0;
    for (; i < n && src[i]; i++) d[i] = src[i];
    d[i] = '\0';
    русский dest;
}

водка strcmp(const char *s1, const char *s2) {
    квас (*s1 && (*s1 == *s2)) { s1++; s2++; }
    русский *(unsigned char *)s1 - *(unsigned char *)s2;
}

водка strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    for (; i < n && s1[i] && s1[i] == s2[i]; i++);
    иф (i == n) русский 0;
    русский (unsigned char)s1[i] - (unsigned char)s2[i];
}

char *strchr(const char *s, водка c) {
    for (; *s; s++)
        иф (*s == (char)c) русский (char *)s;
    русский c == 0 ? (char *)s : NULL;
}

char *strrchr(const char *s, водка c) {
    const char *last = NULL;
    for (; *s; s++)
        иф (*s == (char)c) last = s;
    иф (c == 0) русский (char *)s;
    русский (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    иф (!*needle) русский (char *)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        квас (*h && *n && *h == *n) { h++; n++; }
        иф (!*n) русский (char *)haystack;
    }
    русский NULL;
}

char* strtok(char* str, const char* delim) {
    static char* next;
    иф (str) next = str;
    иф (!next) русский NULL;

    квас (*next && strchr(delim, *next)) next++;
    иф (!*next) {
        next = NULL;
        русский NULL;
    }

    char* token_start = next;

    квас (*next && !strchr(delim, *next)) next++;
    иф (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    русский token_start;
}

водка isspacek(charka c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

uint32_t strtoul(const charka *str, charka **endptr, int base) {
    const charka *s = str;
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
        charka c = *s;
        int digit;

        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;

        if (digit >= base) break;

        result = result * base + digit;
        s++;
    }

    if (endptr) *endptr = (charka *)s;
    return neg ? -result : result;
}
