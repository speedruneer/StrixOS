#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) mode_info {
    uint16_t ModeAttributes;
    uint8_t  WinAAttributes;
    uint8_t  WinBAttributes;
    uint16_t WinGranularity;
    uint16_t WinSize;
    uint16_t WinASegment;
    uint16_t WinBSegment;
    uint32_t WinFuncPtr;
    uint16_t BytesPerScanLine;
    uint16_t XResolution;
    uint16_t YResolution;
    uint8_t  XCharSize;
    uint8_t  YCharSize;
    uint8_t  NumberOfPlanes;
    uint8_t  BitsPerPixel;
    uint8_t  NumberOfBanks;
    uint8_t  MemoryModel;
    uint8_t  BankSize;
    uint8_t  NumberOfImagePages;
    uint8_t  Reserved_page;
    uint8_t  RedMaskSize;
    uint8_t  RedMaskPos;
    uint8_t  GreenMaskSize;
    uint8_t  GreenMaskPos;
    uint8_t  BlueMaskSize;
    uint8_t  BlueMaskPos;
    uint8_t  ReservedMaskSize;
    uint8_t  ReservedMaskPos;
    uint8_t  DirectColorModeInfo;
    uint32_t PhysBasePtr;
    uint32_t OffScreenMemOffset;
    uint16_t OffScreenMemSize;
} mode_info_t;

// Basic drawing functions
void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void rectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int width);
void clear_screen(uint8_t r, uint8_t g, uint8_t b);

// Optional helpers
void draw_hline(int x, int y, int w, uint8_t r, uint8_t g, uint8_t b);
void draw_vline(int x, int y, int h, uint8_t r, uint8_t g, uint8_t b);
void circle(int cx, int cy, int radius, uint8_t r, uint8_t g, uint8_t b);

extern mode_info_t vesa_mode_info;