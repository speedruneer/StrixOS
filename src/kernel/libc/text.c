// text.c -- framebuffer text output + minimal printf family
// Replaces the toy formatter with something actually useful.
// Supports: %c %s %d %i %u %x %X %p, width and zero-padding (e.g. %02X), %#x, l and ll.

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <text.h>
#include <vesa.h>  // for set_pixel, clear_screen, vesa_mode_info

/* Font provided elsewhere */
uint8_t font[256][16];

/* Cursor */
int cursor_x = 0;
int cursor_y = 0;

/* Colors */
static uint8_t fg_r = 255, fg_g = 255, fg_b = 255;
static uint8_t bg_r = 0, bg_g = 0, bg_b = 0;

/* Max rows/cols computed from VESA */
static int max_cols = 0;
static int max_rows = 0;

/* Screen buffer (fixed max) */
#define MAX_ROWS 128
#define MAX_COLS 256
static char screen_buffer[MAX_ROWS * MAX_COLS];

/* Forward declarations */
void redraw_from_buffer(void);
void update_max(void);

/* Helper: update max rows/cols according to VESA resolution */
void update_max(void) {
    if (vesa_mode_info.XResolution > 0 && vesa_mode_info.YResolution > 0) {
        int cols = vesa_mode_info.XResolution / CHAR_WIDTH;
        int rows = vesa_mode_info.YResolution / CHAR_HEIGHT;
        if (cols <= 0) cols = 80;
        if (rows <= 0) rows = 25;
        if (cols > MAX_COLS) cols = MAX_COLS;
        if (rows > MAX_ROWS) rows = MAX_ROWS;
        /* If size changed, don't blow away buffer contents; keep content but adjust limits */
        max_cols = cols;
        max_rows = rows;
    } else {
        max_cols = 80;
        max_rows = 25;
    }
}

/* Draw one character at grid coordinates */
void draw_char(int cx, int cy, char c) {
    update_max();
    if (cx < 0 || cy < 0 || cx >= max_cols || cy >= max_rows) return;

    unsigned char uc = (unsigned char)c;
    if (uc < 32 || uc > 126) uc = '?';

    /* store in buffer */
    screen_buffer[cy * max_cols + cx] = (char)uc;

    const uint8_t *bitmap = font[uc];
    int px = cx * CHAR_WIDTH;
    int py = cy * CHAR_HEIGHT;

    for (int row = 0; row < CHAR_HEIGHT; ++row) {
        uint8_t bits = bitmap[row]; /* font rows usually 8-bit wide */
        for (int col = 0; col < CHAR_WIDTH; ++col) {
            if (bits & (1u << (7 - col))) {
                set_pixel(px + col, py + row, fg_r, fg_g, fg_b);
            } else {
                set_pixel(px + col, py + row, bg_r, bg_g, bg_b);
            }
        }
    }
}

/* Redraw entire screen from buffer */
void redraw_from_buffer(void) {
    update_max();
    clear_screen(bg_r, bg_g, bg_b);
    for (int y = 0; y < max_rows; ++y) {
        for (int x = 0; x < max_cols; ++x) {
            char ch = screen_buffer[y * max_cols + x];
            /* if buffer contains '\0' (uninitialized), treat as space */
            if (ch == '\0') ch = ' ';
            draw_char(x, y, ch);
        }
    }
}

/* Scroll up one line */
static void scroll_up(void) {
    update_max();
    /* move each row up by one */
    for (int y = 1; y < max_rows; ++y) {
        memcpy(&screen_buffer[(y - 1) * max_cols],
               &screen_buffer[y * max_cols],
               (size_t)max_cols);
    }
    /* clear last row */
    for (int x = 0; x < max_cols; ++x) {
        screen_buffer[(max_rows - 1) * max_cols + x] = ' ';
    }
    /* redraw whole screen */
    redraw_from_buffer();
    cursor_y = max_rows - 1;
    if (cursor_x >= max_cols) cursor_x = max_cols - 1;
}

/* Handle newline and scrolling */
static void newline_advance(void) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= max_rows) {
        scroll_up();
    }
}

/* Print string to buffer + framebuffer */
void print(const char *s) {
    if (!s) return;
    update_max();
    while (*s) {
        if (*s == '\n') {
            newline_advance();
        } else if (*s == '\r') {
            cursor_x = 0;
        } else if (*s == '\b') {
            if (cursor_x > 0) {
                cursor_x--;
            } else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = max_cols - 1;
            }
            draw_char(cursor_x, cursor_y, ' ');
        } else {
            draw_char(cursor_x, cursor_y, *s);
            cursor_x++;
            if (cursor_x >= max_cols) newline_advance();
        }
        s++;
    }
}

/* Clear screen text (buffer + framebuffer) */
void clear_screen_text(void) {
    update_max();
    clear_screen(bg_r, bg_g, bg_b);
    for (int y = 0; y < max_rows; ++y) {
        for (int x = 0; x < max_cols; ++x) {
            screen_buffer[y * max_cols + x] = ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

/* Set text/background color */
void set_text_color(uint8_t fr, uint8_t fg, uint8_t fb,
                    uint8_t br, uint8_t bg, uint8_t bb) {
    fg_r = fr; fg_g = fg; fg_b = fb;
    bg_r = br; bg_g = bg; bg_b = bb;
    /* redraw with new colors */
    redraw_from_buffer();
}

/* Initialize text subsystem - call this once after VESA is ready */
void text_init(void) {
    update_max();
    /* fill buffer with spaces */
    for (int i = 0; i < MAX_ROWS * MAX_COLS; ++i) screen_buffer[i] = ' ';
    clear_screen(bg_r, bg_g, bg_b);
    redraw_from_buffer();
}

/* ---------------- number -> string helpers ---------------- */

/* convert unsigned long (32-bit) to string (no padding), returns length */
static int ulltoa(unsigned long v, unsigned int base, int uppercase, char *out, size_t out_sz) {
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[33]; // 32-bit max in binary + null
    int pos = 0;

    if (v == 0) {
        if (out_sz > 1) { out[0] = '0'; out[1] = '\0'; return 1; }
        return 0;
    }

    while (v && pos < (int)sizeof(tmp)) {
        tmp[pos++] = digits[v % base];
        v /= base;
    }

    int len = 0;
    if (pos > 0) {
        for (int i = pos - 1; i >= 0 && (size_t)len + 1 < out_sz; --i) {
            out[len++] = tmp[i];
        }
    }

    if (out_sz > 0) out[len < (int)out_sz ? len : out_sz - 1] = '\0';
    return len;
}

/* convert signed long long to string, returns length */
static int slltoa(long v, int base, char *out, size_t out_sz) {
    if (v < 0) {
        if (out_sz < 2) return 0;
        out[0] = '-';
        int l = ulltoa((unsigned long)(-v), (unsigned)base, 0, out + 1, out_sz > 1 ? out_sz - 1 : 0);
        return 1 + l;
    } else {
        return ulltoa((unsigned long)v, (unsigned)base, 0, out, out_sz);
    }
}

/* Print a formatted buffer with padding and flags */
static void emit_padded(const char *buf, int blen, int width, char pad, int left) {
    if (width <= blen) {
        /* just print */
        print(buf);
        return;
    }
    int padcnt = width - blen;
    if (!left) {
        for (int i = 0; i < padcnt; ++i) {
            char p[2] = {pad, 0};
            print(p);
        }
        print(buf);
    } else {
        print(buf);
        for (int i = 0; i < padcnt; ++i) {
            char p[2] = {' ', 0};
            print(p);
        }
    }
}

/* ---------------- vprintf implementation ---------------- */

static int vprintf_internal(const char *fmt, va_list ap) {
    int written = 0;
    char tmpbuf[128];

    while (*fmt) {
        if (*fmt != '%') {
            char c[2] = {*fmt, 0};
            print(c);
            written++;
            fmt++;
            continue;
        }
        fmt++; /* skip '%' */

        /* parse flags */
        int left = 0;
        int plus = 0; (void)plus;
        int space_flag = 0; (void)space_flag;
        int zero = 0;
        int alt = 0;
        while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '0' || *fmt == '#') {
            if (*fmt == '-') left = 1;
            else if (*fmt == '0') zero = 1;
            else if (*fmt == '#') alt = 1;
            fmt++;
        }

        /* width */
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        /* length modifiers */
        int length = 0; /* 0=default, 1=l, 2=ll */
        if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { length = 2; fmt++; } else length = 1;
        }

        /* specifier */
        char spec = *fmt ? *fmt : '\0';
        if (!spec) break;

        switch (spec) {
            case 'c': {
                int ch = va_arg(ap, int);
                char out[2] = {(char)ch, 0};
                emit_padded(out, 1, width, zero ? '0' : ' ', left);
                written += (width > 1) ? width : 1;
                break;
            }
            case 's': {
                const char *s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                int len = (int)strlen(s);
                emit_padded(s, len, width, ' ', left);
                written += (width > len) ? width : len;
                break;
            }
            case 'd':
            case 'i': {
                long long v;
                if (length == 2) v = va_arg(ap, long long);
                else if (length == 1) v = va_arg(ap, long);
                else v = va_arg(ap, int);
                int len = slltoa(v, 10, tmpbuf, sizeof(tmpbuf));
                emit_padded(tmpbuf, len, width, zero ? '0' : ' ', left);
                written += (width > len) ? width : len;
                break;
            }
            case 'u': {
                unsigned long long uv;
                if (length == 2) uv = va_arg(ap, unsigned long long);
                else if (length == 1) uv = va_arg(ap, unsigned long);
                else uv = va_arg(ap, unsigned int);
                int len = (int)ulltoa(uv, 10, 0, tmpbuf, sizeof(tmpbuf));
                emit_padded(tmpbuf, len, width, zero ? '0' : ' ', left);
                written += (width > len) ? width : len;
                break;
            }
            case 'x':
            case 'X': {
                unsigned long long uv;
                if (length == 2) uv = va_arg(ap, unsigned long long);
                else if (length == 1) uv = va_arg(ap, unsigned long);
                else uv = va_arg(ap, unsigned int);
                int uppercase = (spec == 'X');
                int len = (int)ulltoa(uv, 16, uppercase, tmpbuf, sizeof(tmpbuf));
                if (alt && uv != 0) {
                    /* prefix */
                    if (!zero) {
                        /* print prefix then value */
                        if (uppercase) print("0X"); else print("0x");
                        written += 2;
                        emit_padded(tmpbuf, len, width - 2, zero ? '0' : ' ', left);
                        written += (width > len + 2) ? width - 2 : len;
                    } else {
                        /* zero-padding and prefix: prefix emitted before padding */
                        if (uppercase) print("0X"); else print("0x");
                        written += 2;
                        for (int i = 0; i < width - len - 2; ++i) { char z[2] = {'0',0}; print(z); written++; }
                        print(tmpbuf); written += len;
                    }
                } else {
                    emit_padded(tmpbuf, len, width, zero ? '0' : ' ', left);
                    written += (width > len) ? width : len;
                }
                break;
            }
            case 'p': {
                void *ptr = va_arg(ap, void*);
                unsigned long long uv = (unsigned long long)(uintptr_t)ptr;
                int len = (int)ulltoa(uv, 16, 0, tmpbuf, sizeof(tmpbuf));
                /* pointer typically shown with 0x prefix, pad to pointer width if width given and zero flag */
                if (zero && width > 0) {
                    /* will produce like 0x00.. */
                    print("0x"); written += 2;
                    for (int i = 0; i < width - 2 - len; ++i) { char z[2] = {'0',0}; print(z); written++; }
                    print(tmpbuf); written += len;
                } else {
                    char out_with_prefix[130];
                    int plen = 0;
                    out_with_prefix[plen++] = '0';
                    out_with_prefix[plen++] = 'x';
                    for (int i = 0; i < len && plen + 1 < (int)sizeof(out_with_prefix); ++i) {
                        out_with_prefix[plen++] = tmpbuf[i];
                    }
                    out_with_prefix[plen] = '\0';
                    emit_padded(out_with_prefix, plen, width, ' ', left);
                    written += (width > plen) ? width : plen;
                }
                break;
            }
            case '%': {
                print("%"); written++; break;
            }
            default: {
                /* unknown: print as-is */
                char out[3] = {'%', spec, 0};
                print(out);
                written += 2;
                break;
            }
        } /* switch */

        fmt++; /* move past specifier */
    } /* while fmt */

    return written;
}

/* printf / println wrappers */
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf_internal(fmt, ap);
    va_end(ap);
    return ret;
}

int println(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf_internal(fmt, ap);
    va_end(ap);
    print("\n");
    return ret + 1;
}

/* Minimal stack protector stub (keeps the kernel linking happy) */
void __stack_chk_fail_local(void) { for (;;) asm volatile("hlt"); }
