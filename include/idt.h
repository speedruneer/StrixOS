#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define PIC1          0x20
#define PIC2          0xA0
#define PIC1_COMMAND  PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND  PIC2
#define PIC2_DATA    (PIC2+1)
#define PIC_EOI       0x20

#define ICW1          0x11
#define ICW4          0x01

#define IDT_ENTRIES 256
#define PIC1 0x20
#define PIC2 0xA0
#define PIC_EOI 0x20

#define KBD_DATA     0x60

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_t;

extern void load_idt(idt_t(*));
void set_idt(idt_entry_t* idt, int n, uint32_t handler, uint16_t sel, uint8_t flags);
void init_idt(idt_entry_t* idt);
void pic_init(void);

// ISR function pointer type
typedef void (*isr_t)(void);
extern isr_t irq_handlers[16];  // user-provided IRQ handlers

#endif
