#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

/* External font array provided by BIOS/bootloader */
extern uint8_t font[256][16];

/* Character dimensions */
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

/* Cursor position */
extern int cursor_x;
extern int cursor_y;

/* Public API */
void update_max(void);
void draw_char(int cx, int cy, char c);
void print(const char *s);
void clear_screen_text(void);
void set_text_color(uint8_t fr, uint8_t fg, uint8_t fb,
                    uint8_t br, uint8_t bg, uint8_t bb);

void text_init(void);
#endif /* TEXT_H */
