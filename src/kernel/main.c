#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vesa.h>
#include <text.h>
#include <function.h>
#include <memutils.h>
#include <ata.h>
#include <asm.h>
#include <idt.h>

idt_entry_t idt[256];

// ---------------- Keyboard Layouts ----------------

// QWERTY
static const char layout_en[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = ';', [0x28] = '\'',
    [0x29] = '`',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x39] = ' ', [0x1C] = '\n', [0x0E] = '\b'
};

// QWERTY shifted
static const char layout_en_shift[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')',
    [0x0C] = '_', [0x0D] = '+',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L', [0x27] = ':', [0x28] = '"',
    [0x29] = '~',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
    [0x33] = '<', [0x34] = '>', [0x35] = '?',
    [0x39] = ' ', [0x1C] = '\n', [0x0E] = '\b'
};

// AZERTY
static const char layout_fr[128] = {
    [0x02] = '&', [0x04] = '"', [0x05] = '\'',
    [0x06] = '(', [0x07] = '-', [0x09] = '_',
    [0x0C] = ')', [0x0D] = '=',
    [0x10] = 'a', [0x11] = 'z', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '^', [0x1B] = '$',
    [0x1E] = 'q', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = 'm',
    [0x2C] = 'w', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n',
    [0x32] = ',', [0x33] = ';', [0x34] = ':', [0x35] = '!',
    [0x39] = ' ', [0x1C] = '\n', [0x0E] = '\b'
};

// AZERTY shifted
static const char layout_fr_shift[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x0D] = '+',
    [0x10] = 'A', [0x11] = 'Z', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P',
    [0x1E] = 'Q', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L', [0x27] = 'M', [0x28] = '%',
    [0x2C] = 'W', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N',
    [0x32] = '?', [0x33] = '.', [0x34] = '/',
    [0x39] = ' ', [0x1C] = '\n', [0x0E] = '\b'
};

// Current active layout pointers
static const char* current_layout = layout_fr;
static const char* current_layout_shift = layout_fr_shift;

extern volatile unsigned char last_scancode;
extern volatile unsigned char last_key_state;

#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36

static char is_shift_pressed = 0;

// Simple shell buffer
#define SHELL_BUF_SIZE 256
char shell_line[SHELL_BUF_SIZE];
size_t shell_pos = 0;

// Forward
void execute_command(const char* line);

// ---------------- Shell main ----------------

void main() {
    set_text_color(255,255,255,0,0,0);
    clear_screen(0,0,0);
    init_font();
    text_init();

    printf("[kernel] Booted\n");
    printf("[kernel] VESA: %ux%u, %u-bit color, %u-byte framebuffer at 0x%08X\n",
        vesa_mode_info.XResolution, vesa_mode_info.YResolution,
        vesa_mode_info.BitsPerPixel,
        vesa_mode_info.BytesPerScanLine * vesa_mode_info.YResolution,
        vesa_mode_info.PhysBasePtr
    );

    new_func((function)malloc, "malloc");
    new_func((function)free, "free");
    new_func((function)new_func, "new_func");
    new_func((function)load_idt, "load_idt");

    init_idt(&idt);

    printf("> ");

    static unsigned char prev_scancode = 0;

    while (1) {
        unsigned char sc = last_scancode;
        if (sc == prev_scancode) continue;
        prev_scancode = sc;

        int released = sc & 0x80;
        unsigned char key = sc & 0x7F;

        if (key == SC_LSHIFT || key == SC_RSHIFT) {
            is_shift_pressed = !released;
            continue;
        }

        if (released) continue;

        char c;
        if (is_shift_pressed)
            c = current_layout_shift[key];
        else
            c = current_layout[key];

        if (c) {
            if (c == '\n') {
                shell_line[shell_pos] = 0;
                printf("\n");
                execute_command(shell_line);
                shell_pos = 0;
                printf("> ");
            } else if (c == '\b') {
                if (shell_pos > 0) {
                    shell_pos--;
                    printf("\b");
                }
            } else if (shell_pos < SHELL_BUF_SIZE - 1) {
                shell_line[shell_pos++] = c;
                printf("%c", c);
            }
        }
    }

    return;
}

// ---------------------- Minimal Shell -----------------------

char current_path[512];

void execute_command(const char* line) {
    if (!line || !*line) return;

    if (strncmp(line, "echo ", 5) == 0) {
        printf("%s\n", line + 5);
    } else if (strcmp(line, "shutdown") == 0) {
        volatile int8_t* reboot_byte = (int8_t*)0x8027;
        *reboot_byte = 0x76;
        clear_screen_text();
        printf("REBOOTING\n");
        for (volatile int i=0;i<2000000000;i+=10){i++;i--;}
        void *ptr = (void*)0; volatile size_t sz = 0xFFFFFFFFu; memset(ptr, 0, sz);
    } else if (strcmp(line, "clear") == 0) {
        clear_screen_text();
    } else if (strcmp(line, "time") == 0) {
        printf("FakeOS time: who cares\n");
    } else if (strcmp(line, "circle") == 0) {
        circle(300, 300, 100, 255, 255, 255);
    } else if (strcmp(line, "loadkeys fr") == 0) {
        current_layout = layout_fr;
        current_layout_shift = layout_fr_shift;
        printf("Keyboard layout set to FR (AZERTY)\n");
    } else if (strcmp(line, "loadkeys en") == 0) {
        current_layout = layout_en;
        current_layout_shift = layout_en_shift;
        printf("Keyboard layout set to EN (QWERTY)\n");
    } else if (strncmp(line, "peek ", 5) == 0) {
        unsigned int addr = strtoul(line+5, NULL, 16);
        printf("Value at 0x%08X: 0x%02X\n", addr, *((unsigned char*)addr));
    } else if (strncmp(line, "memdump ", 8) == 0) {
        char *endptr;
        unsigned int addr = strtoul(line + 8, &endptr, 0);
        unsigned int size = strtoul(endptr, NULL, 0);

        if (size == 0) size = 16; // default minimal dump

        memdump((void*)addr, size);
    } else if (strncmp(line, "poke ", 5) == 0) {
        char *endptr;
        unsigned int addr = strtoul(line + 5, &endptr, 0);
        unsigned int val  = strtoul(endptr, NULL, 0);
        *((unsigned char*)addr) = (unsigned char)val;
        printf("Wrote 0x%02X to 0x%08X\n", val, addr);
    } else if (strcmp(line, "nuke") == 0) {
        volatile int8_t* reboot_byte = (int8_t*)0x8027;
        *reboot_byte = 0x00; // guarantee we don't reboot
        clear_screen_text();
        printf("NUKING\n");
        for (volatile int i=0;i<2000000000;i+=10){i++;i--;}
        void *ptr = (void*)0; volatile size_t sz = 0xFFFFFFFFu; memset(ptr, 0, sz);
    } else if (strncmp(line, "ataread ", 8) == 0) {
        char *endptr;
        unsigned int lba = strtoul(line + 8, &endptr, 0);
        unsigned int count = strtoul(endptr, &endptr, 0);
        unsigned int addr = 0x7C00; // default
        if (*endptr) addr = strtoul(endptr, NULL, 0);

        int ret = ATA_READ_28((void*)addr, lba, (uint8_t)count);
        if (ret == 0)
            printf("Read %u sectors from LBA %u into 0x%08X\n", count, lba, addr);
        else
            printf("ATA read failed (LBA %u, count %u)\n", lba, count);
    } else if (strncmp(line, "atawrite ", 9) == 0) {
        char *endptr;
        unsigned int lba = strtoul(line + 9, &endptr, 0);
        unsigned int size = strtoul(endptr, &endptr, 0);
        unsigned int addr = 0x7C00; // default
        if (*endptr) addr = strtoul(endptr, NULL, 0);

        int ret = ATA_WRITE_28((void*)addr, lba, (uint8_t)size);
        if (ret == 0)
            printf("Wrote %u sectors from 0x%08X to LBA %u\n", size, addr, lba);
        else
            printf("ATA write failed (LBA %u, count %u)\n", lba, size);
    } else {
        printf("Unknown command: %s\n", line);
    }
}