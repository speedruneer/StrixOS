#include <vesa.h>
#include <stddef.h>

extern mode_info_t vesa_mode_info;

static inline void put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= vesa_mode_info.XResolution || y < 0 || y >= vesa_mode_info.YResolution)
        return;

    uint8_t *fb = (uint8_t*)(uintptr_t)vesa_mode_info.PhysBasePtr;
    size_t offset = y * vesa_mode_info.BytesPerScanLine + x * (vesa_mode_info.BitsPerPixel / 8);

    switch (vesa_mode_info.BitsPerPixel) {
        case 16: {
            // Assume 5-5-5 RGB
            uint16_t color16 = ((r >> 3) << vesa_mode_info.RedMaskPos) |
                               ((g >> 3) << vesa_mode_info.GreenMaskPos) |
                               ((b >> 3) << vesa_mode_info.BlueMaskPos);
            *((uint16_t*)(fb + offset)) = color16;
            break;
        }
        case 24: {
            fb[offset + vesa_mode_info.RedMaskPos / 8]   = r;
            fb[offset + vesa_mode_info.GreenMaskPos / 8] = g;
            fb[offset + vesa_mode_info.BlueMaskPos / 8]  = b;
            break;
        }
        case 32: {
            uint32_t color32 = (r << vesa_mode_info.RedMaskPos) |
                               (g << vesa_mode_info.GreenMaskPos) |
                               (b << vesa_mode_info.BlueMaskPos);
            *((uint32_t*)(fb + offset)) = color32;
            break;
        }
    }
}

void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    put_pixel(x, y, r, g, b);
}

void draw_hline(int x, int y, int w, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < w; i++) set_pixel(x + i, y, r, g, b);
}

void draw_vline(int x, int y, int h, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < h; i++) set_pixel(x, y + i, r, g, b);
}

void rectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    for (int j = 0; j < h; j++) draw_hline(x, y + j, w, r, g, b);
}

void line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int width) {
    int dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 > y0) ? y1 - y0 : y0 - y1;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        for (int wx = -width/2; wx <= width/2; wx++)
            for (int wy = -width/2; wy <= width/2; wy++)
                set_pixel(x0 + wx, y0 + wy, r, g, b);

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void clear_screen(uint8_t r, uint8_t g, uint8_t b) {
    for (int y = 0; y < vesa_mode_info.YResolution; y++) {
        for (int x = 0; x < vesa_mode_info.XResolution; x++)
            set_pixel(x, y, r, g, b);
    }
}

void circle(int cx, int cy, int radius, uint8_t r, uint8_t g, uint8_t b) {
    int x = radius, y = 0;
    int err = 0;

    while (x >= y) {
        set_pixel(cx + x, cy + y, r, g, b);
        set_pixel(cx + y, cy + x, r, g, b);
        set_pixel(cx - y, cy + x, r, g, b);
        set_pixel(cx - x, cy + y, r, g, b);
        set_pixel(cx - x, cy - y, r, g, b);
        set_pixel(cx - y, cy - x, r, g, b);
        set_pixel(cx + y, cy - x, r, g, b);
        set_pixel(cx + x, cy - y, r, g, b);

        y++;
        if (err <= 0) err += 2*y + 1;
        if (err > 0)  { x--; err -= 2*x + 1; }
    }
}
