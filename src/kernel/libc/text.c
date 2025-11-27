// text.c -- framebuffer text output + minimal printf family
// Multi-byte mapping: single byte 0..127 = glyph 0..127
// if byte >= 128 then use (b1,b2) -> index = 128 + (b1-128)*256 + b2
// Max glyphs = 32896, each glyph = 16 bytes => 32896*16 = 526336 bytes

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <text.h>
#include <vesa.h>  // expects set_pixel, clear_screen, vesa_mode_info, CHAR_WIDTH, CHAR_HEIGHT

/* ——— Font data ——— */
/* font: pointer to array of glyphs; each glyph = 16 bytes (rows) */
static uint8_t (*font)[16] = NULL;

/* base_font: 256 glyphs x 16 bytes each (BIOS / fallback) */
uint8_t base_font[256][16];

/* constants derived from your mapping */
#define SINGLE_BYTE_LIMIT 128U
#define EXT_BLOCKS 128U        /* 128 possible b1 values (128..255) */
#define EXT_BLOCK_WIDTH 256U   /* b2 = 0..255 */
#define NUM_GLYPHS (SINGLE_BYTE_LIMIT + (EXT_BLOCKS * EXT_BLOCK_WIDTH)) /* 32896 */
#define GLYPH_BYTES 16U
#define FONT_BYTES (NUM_GLYPHS * GLYPH_BYTES) /* 526336 */

/* Cursor */
int cursor_x = 0;
int cursor_y = 0;

/* Colors */
static uint8_t fg_r = 255, fg_g = 255, fg_b = 255;
static uint8_t bg_r = 0, bg_g = 0, bg_b = 0;

/* Max rows/cols computed from VESA */
static int max_cols = 0;
static int max_rows = 0;

/* Screen buffer now holds glyph indices (uint16_t is enough: 0..32895) */
#define MAX_ROWS 128
#define MAX_COLS 256
static uint16_t screen_buffer[MAX_ROWS * MAX_COLS];

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
        max_cols = cols;
        max_rows = rows;
    } else {
        max_cols = 80;
        max_rows = 25;
    }
}

/* low-level: draw glyph at framebuffer coordinates (cx,cy grid) */
static void draw_glyph_at_cell(uint16_t glyph_index, int cx, int cy) {
    if (!font) return; /* nothing loaded */
    if (glyph_index >= NUM_GLYPHS) glyph_index = (uint16_t)'?'; /* fallback to '?' index in first 128 */

    const uint8_t *bitmap = font[glyph_index];
    const int px = cx * CHAR_WIDTH;
    const int py = cy * CHAR_HEIGHT;

    for (int row = 0; row < CHAR_HEIGHT; ++row) {
        uint8_t bits = bitmap[row];
        for (int col = 0; col < CHAR_WIDTH; ++col) {
            if (bits & (1u << (7 - col))) {
                set_pixel(px + col, py + row, fg_r, fg_g, fg_b);
            } else {
                set_pixel(px + col, py + row, bg_r, bg_g, bg_b);
            }
        }
    }
}

/* Draw one character at grid coordinates (accepts codepoint index) */
void draw_char_cell(int cx, int cy, uint16_t code) {
    update_max();
    if (cx < 0 || cy < 0 || cx >= max_cols || cy >= max_rows) return;
    /* store in buffer */
    screen_buffer[cy * max_cols + cx] = code;
    draw_glyph_at_cell(code, cx, cy);
}

/* Redraw entire screen from buffer */
void redraw_from_buffer(void) {
    update_max();
    clear_screen(bg_r, bg_g, bg_b);
    for (int y = 0; y < max_rows; ++y) {
        for (int x = 0; x < max_cols; ++x) {
            uint16_t glyph = screen_buffer[y * max_cols + x];
            /* treat 0 as space if uninitialized */
            if (glyph == 0) glyph = (uint16_t)' ';
            draw_glyph_at_cell(glyph, x, y);
        }
    }
}

/* Scroll up one line */
static void scroll_up(void) {
    update_max();
    size_t row_bytes = (size_t)max_cols * sizeof(screen_buffer[0]);
    for (int y = 1; y < max_rows; ++y) {
        memcpy(&screen_buffer[(y - 1) * max_cols],
               &screen_buffer[y * max_cols],
               row_bytes);
    }
    /* clear last row */
    for (int x = 0; x < max_cols; ++x) {
        screen_buffer[(max_rows - 1) * max_cols + x] = (uint16_t)' ';
    }
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

/* Helper: decode the custom 1/2-byte encoding from a byte stream.
   - s points to the current byte. This function consumes bytes via *s_ptr.
   - Returns glyph index and advances (*s_ptr) appropriately.
*/
static uint16_t decode_glyph_from_bytes(const char **s_ptr) {
    const unsigned char *s = (const unsigned char *)(*s_ptr);
    if (!s || *s == 0) return (uint16_t)'?';
    unsigned char b1 = *s++;
    if (b1 < SINGLE_BYTE_LIMIT) {
        *s_ptr = (const char *)s;
        return (uint16_t)b1;
    } else {
        /* Need a second byte. If missing, return '?' and do not overrun */
        unsigned char b2 = 0;
        if (*s) {
            b2 = *s++;
            *s_ptr = (const char *)s;
        } else {
            /* malformed: missing second byte -> fallback to '?' and do not advance further */
            *s_ptr = (const char *)s;
            return (uint16_t)'?';
        }
        uint32_t index = SINGLE_BYTE_LIMIT + ((uint32_t)(b1 - SINGLE_BYTE_LIMIT) * EXT_BLOCK_WIDTH) + (uint32_t)b2;
        if (index >= NUM_GLYPHS) return (uint16_t)'?';
        return (uint16_t)index;
    }
}

void print(const char *s) {
    if (!s) return;
    update_max();
    while (*s) {
        if (*s == '\n') {
            newline_advance();
            s++;
            continue;
        } else if (*s == '\r') {
            cursor_x = 0;
            s++;
            continue;
        } else if (*s == '\b') {
            if (cursor_x > 0) {
                cursor_x--;
            } else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = max_cols - 1;
            }
            draw_char_cell(cursor_x, cursor_y, (uint16_t)' ');
            s++;
            continue;
        } else if (*s == '/') { // handle escape sequences
            s++;
            if (*s == '0') { // null byte
                draw_char_cell(cursor_x, cursor_y, 0);
                cursor_x++;
                s++;
                continue;
            } else if (*s == '%') { // \%d
                s++;
                if (*s == 'd') {
                    s++;
                    if (*s >= '0' && *s <= '9') {
                        uint8_t val = *s - '0';
                        draw_char_cell(cursor_x, cursor_y, val);
                        cursor_x++;
                        s++;
                    }
                    continue;
                }
            } else if (*s == '\\') { // literal backslash
                draw_char_cell(cursor_x, cursor_y, '\\');
                cursor_x++;
                s++;
                continue;
            }
        }

        uint16_t glyph = decode_glyph_from_bytes(&s);
        draw_char_cell(cursor_x, cursor_y, glyph);
        cursor_x++;
        if (cursor_x >= max_cols) newline_advance();
    }
}

/* Helper to set a glyph */
void set_glyph(uint32_t glyph_index, const uint8_t glyph[16]) {
    if (!font) return;
    if (glyph_index >= NUM_GLYPHS) return;
    memcpy(font[glyph_index], glyph, GLYPH_BYTES);
}

/* Initialize font: allocate, zero, copy base_font and fill rest with '?' */
void init_font(void) {
    /* allocate */
    font = malloc((size_t)NUM_GLYPHS * GLYPH_BYTES);
    if (!font) {
        printf("init_font: malloc failed\n");
        return;
    }

    /* zero everything (makes undefined glyphs blank until we set '?') */
    memset(font, 0, (size_t)NUM_GLYPHS * GLYPH_BYTES);

    /* copy base 256 glyphs at start */
    memcpy(font, base_font, 256 * GLYPH_BYTES);

    /* determine index of '?' within base_font */
    unsigned char qidx = (unsigned char)'?';
    const uint8_t *qglyph = base_font[qidx];

    /* fill extended glyphs with '?' glyph */
    for (uint32_t i = 256; i < NUM_GLYPHS; ++i) {
        memcpy(font[i], qglyph, GLYPH_BYTES);
    }
}

/* Clear screen text (buffer + framebuffer) */
void clear_screen_text(void) {
    update_max();
    clear_screen(bg_r, bg_g, bg_b);
    for (int y = 0; y < max_rows; ++y) {
        for (int x = 0; x < max_cols; ++x) {
            screen_buffer[y * max_cols + x] = (uint16_t)' ';
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
    redraw_from_buffer();
}

/* Initialize text subsystem - call this once after VESA is ready */
void text_init(void) {
    clear_screen_text();
    update_max();
    /* fill buffer with spaces */
    for (int i = 0; i < MAX_ROWS * MAX_COLS; ++i) screen_buffer[i] = (uint16_t)' ';
    clear_screen(bg_r, bg_g, bg_b);
    redraw_from_buffer();
}

/* ---------------- number -> string helpers ---------------- */

/* convert unsigned long (32-bit) to string (no padding), returns length */
static int ulltoa(unsigned long v, unsigned int base, int uppercase, char *out, size_t out_sz) {
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[65]; // safe for up to 64-bit
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

/* convert signed long to string, returns length */
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
    char tmpbuf[256];

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
                long v;
                if (length == 2) v = va_arg(ap, long);
                else if (length == 1) v = va_arg(ap, long);
                else v = va_arg(ap, int);
                int len = slltoa(v, 10, tmpbuf, sizeof(tmpbuf));
                emit_padded(tmpbuf, len, width, zero ? '0' : ' ', left);
                written += (width > len) ? width : len;
                break;
            }
            case 'u': {
                unsigned long uv;
                if (length == 2) uv = va_arg(ap, unsigned long);
                else if (length == 1) uv = va_arg(ap, unsigned long);
                else uv = va_arg(ap, unsigned int);
                int len = (int)ulltoa(uv, 10, 0, tmpbuf, sizeof(tmpbuf));
                emit_padded(tmpbuf, len, width, zero ? '0' : ' ', left);
                written += (width > len) ? width : len;
                break;
            }
            case 'x':
            case 'X': {
                unsigned long uv;
                if (length == 2) uv = va_arg(ap, unsigned long);
                else if (length == 1) uv = va_arg(ap, unsigned long);
                else uv = va_arg(ap, unsigned int);
                int uppercase = (spec == 'X');
                int len = (int)ulltoa(uv, 16, uppercase, tmpbuf, sizeof(tmpbuf));
                if (alt && uv != 0) {
                    if (!zero) {
                        if (uppercase) print("0X"); else print("0x");
                        written += 2;
                        emit_padded(tmpbuf, len, width - 2, zero ? '0' : ' ', left);
                        written += (width > len + 2) ? width - 2 : len;
                    } else {
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
                unsigned long uv = (unsigned long)(uintptr_t)ptr;
                int len = (int)ulltoa(uv, 16, 0, tmpbuf, sizeof(tmpbuf));
                if (zero && width > 0) {
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
