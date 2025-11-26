#include <idt.h>
#include <stdio.h>   // for printf, can be replaced with your kernel print
#include <cyrillic.h>   // for printf, can be replaced with your kernel print
#include <asm.h>     // for outb
#include <vesa.h>     // for outb
#include <text.h>     // for outb


static void print_registers_state(void) {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp;
    uint32_t eip, cs, ds, es, fs, gs, ss, eflags;

    asm volatile(
        "mov %%eax, %0\n\t"
        "mov %%ebx, %1\n\t"
        "mov %%ecx, %2\n\t"
        "mov %%edx, %3\n\t"
        "mov %%esi, %4\n\t"
        "mov %%edi, %5\n\t"
        "mov %%ebp, %6\n\t"
        "mov %%esp, %7\n\t"
        "pushf\n\t"
        "pop %15\n\t"
        "mov %%cs, %8\n\t"
        "mov %%ds, %9\n\t"
        "mov %%es, %10\n\t"
        "mov %%fs, %11\n\t"
        "mov %%gs, %12\n\t"
        "mov %%ss, %13\n\t"
        "call 1f\n\t"
        "1: pop %14\n\t"
        : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx),
          "=m"(esi), "=m"(edi), "=m"(ebp), "=m"(esp),
          "=m"(cs), "=m"(ds), "=m"(es), "=m"(fs),
          "=m"(gs), "=m"(ss), "=m"(eip), "=m"(eflags)
        :
        : "memory"
    );

    printf("Registers state:\n");
    printf("EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n", eax, ebx, ecx, edx);
    printf("ESI=%08X EDI=%08X EBP=%08X ESP=%08X\n", esi, edi, ebp, esp);
    printf("EIP=%08X CS=%04X DS=%04X ES=%04X FS=%04X GS=%04X SS=%04X\n",
           eip, cs, ds, es, fs, gs, ss);
    printf("EFLAGS=%08X\n", eflags);
}

void base_exception(const char* msg) {
    asm volatile ("cli");
    clear_screen(0, 0, 0);
    clear_screen_text();
    printf("Exception Catched: %s Error/Exception\n", msg);
    print_registers_state();
    for (;;) {asm volatile ("hlt");}
}

static void set_idt_entry(idt_entry_t* idt, int n, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = sel;
    idt[n].zero        = 0;
    idt[n].type_attr   = flags;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

static void exception0()  {
    asm volatile ("cli");
    if (*(char*)0x8027 == 0x76) {
        return;
        volatile int x = 1;
        volatile int zero = 0;
        x /= zero;
    }
    clear_screen(0, 0, 0);
    clear_screen_text();
    printf("Exception Catched: Invalid Opcode Error/Exception\n");
    print_registers_state();
    for (;;) {asm volatile ("hlt");}
}
static void exception1()  { base_exception("Debug"); }
static void exception2()  { base_exception("Non-Maskable Interrupt"); }
static void exception3()  { base_exception("Breakpoint"); }
static void exception4()  { base_exception("Overflow"); }
static void exception5()  { base_exception("BOUND Range Exceeded"); }
static void exception6()  {
    asm volatile ("cli");
    if (*(char*)0x8027 == 0x76) {
        return;
        volatile int x = 1;
        volatile int zero = 0;
        x /= zero;
    }
    clear_screen(0, 0, 0);
    clear_screen_text();
    printf("Exception Catched: Invalid Opcode Error/Exception\n");
    print_registers_state();
    for (;;) {asm volatile ("hlt");}
}
static void exception7()  { base_exception("Device Not Available"); }
static void exception8()  { base_exception("Double Fault"); }
static void exception9()  { base_exception("Coprocessor Segment Overrun"); }
static void exception10() { base_exception("Invalid TSS"); }
static void exception11() { base_exception("Segment Not Present"); }
static void exception12() { base_exception("Stack-Segment Fault"); }
static void exception13() { base_exception("General Protection"); }
static void exception14() { base_exception("Page Fault"); }
static void exception15() { base_exception("Reserved"); }
static void exception16() { base_exception("x87 Floating-Point Exception"); }
static void exception17() { base_exception("Alignment Check"); }
static void exception18() { base_exception("Machine Check"); }
static void exception19() { base_exception("SIMD Floating-Point Exception"); }
static void exception20() { base_exception("Virtualization Exception"); }
static void exception21() { base_exception("Reserved"); }
static void exception22() { base_exception("Reserved"); }
static void exception23() { base_exception("Reserved"); }
static void exception24() { base_exception("Reserved"); }
static void exception25() { base_exception("Reserved"); }
static void exception26() { base_exception("Reserved"); }
static void exception27() { base_exception("Reserved"); }
static void exception28() { base_exception("Reserved"); }
static void exception29() { base_exception("Reserved"); }
static void exception30() { base_exception("Security Exception"); }
static void exception31() { base_exception("Reserved"); }

void pic_init() {
    // ICW1
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);

    // ICW2, irq 0 to 7 is mapped to 0x20 to 0x27, irq 8 to F is mapped to 28 to 2F
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // ICW3, connect master pic with slave pic
    outb(PIC1_DATA, 0x4);
    outb(PIC2_DATA, 0x2);

    // ICW4, set x86 mode
    outb(PIC1_DATA, 1);
    outb(PIC2_DATA, 1);

    // clear the mask register
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

extern void isr_stub();

extern void irq0();
extern void irq1();

int timer_reached_end = 0;

// -----------------------------
// IRQ0 (PIT)
// -----------------------------
__attribute__((naked)) void irq0(void)
{
    asm volatile(
        "pusha\n\t"
        "movl $1, timer_reached_end\n\t"
        "movb $0x20, %%al\n\t"
        "outb %%al, $0x20\n\t"
        "popa\n\t"
        "iret\n\t"
        :
        :
        : "al", "memory"
    );
}

// -----------------------------
// IRQ1 (keyboard)
// -----------------------------
volatile unsigned char last_scancode = 0;
volatile unsigned char is_key_pressed = 0;       // 1 = press, 0 = release
volatile unsigned char last_key_state = 0;       // previous state (same meaning)

// masked helpers
#define SC_RELEASE(sc)   ((sc) & 0x80)           // 1 if release
#define SC_MAKE(sc)      (!((sc) & 0x80))        // 1 if press

__attribute__((naked)) void irq1()
{
    asm volatile(
        "pusha\n\t"

        // read scancode
        "inb $0x60, %%al\n\t"
        "movb %%al, last_scancode\n\t"

        // check if release (bit 7 set)
        "testb $0x80, %%al\n\t"
        "jnz 1f\n\t"              // jump if bit 7 set → release

        // key press
        "movb $1, is_key_pressed\n\t"
        "jmp 2f\n\t"

        // key release label
        "1:\n\t"
        "movb $0, is_key_pressed\n\t"

        // done
        "2:\n\t"

        // EOI to PIC
        "movb $0x20, %%al\n\t"
        "outb %%al, $0x20\n\t"

        "popa\n\t"
        "iret\n\t"
        :
        :
        : "al"
    );
}



void init_idt(idt_entry_t* idt) {
    DEBUG_PRINT("[idt] Initializing IDT\n");
    pic_init();
    set_idt_entry(idt, 0,  (uint32_t)exception0,  0x08, 0x8E);
    set_idt_entry(idt, 1,  (uint32_t)exception1,  0x08, 0x8E);
    set_idt_entry(idt, 2,  (uint32_t)exception2,  0x08, 0x8E);
    set_idt_entry(idt, 3,  (uint32_t)exception3,  0x08, 0x8E);
    set_idt_entry(idt, 4,  (uint32_t)exception4,  0x08, 0x8E);
    set_idt_entry(idt, 5,  (uint32_t)exception5,  0x08, 0x8E);
    set_idt_entry(idt, 6,  (uint32_t)exception6,  0x08, 0x8E);
    set_idt_entry(idt, 7,  (uint32_t)exception7,  0x08, 0x8E);
    set_idt_entry(idt, 8,  (uint32_t)exception8,  0x08, 0x8E);
    set_idt_entry(idt, 9,  (uint32_t)exception9,  0x08, 0x8E);
    set_idt_entry(idt, 10, (uint32_t)exception10, 0x08, 0x8E);
    set_idt_entry(idt, 11, (uint32_t)exception11, 0x08, 0x8E);
    set_idt_entry(idt, 12, (uint32_t)exception12, 0x08, 0x8E);
    set_idt_entry(idt, 13, (uint32_t)exception13, 0x08, 0x8E);
    set_idt_entry(idt, 14, (uint32_t)exception14, 0x08, 0x8E);
    set_idt_entry(idt, 15, (uint32_t)exception15, 0x08, 0x8E);
    set_idt_entry(idt, 16, (uint32_t)exception16, 0x08, 0x8E);
    set_idt_entry(idt, 17, (uint32_t)exception17, 0x08, 0x8E);
    set_idt_entry(idt, 18, (uint32_t)exception18, 0x08, 0x8E);
    set_idt_entry(idt, 19, (uint32_t)exception19, 0x08, 0x8E);
    set_idt_entry(idt, 20, (uint32_t)exception20, 0x08, 0x8E);
    set_idt_entry(idt, 21, (uint32_t)exception21, 0x08, 0x8E);
    set_idt_entry(idt, 22, (uint32_t)exception22, 0x08, 0x8E);
    set_idt_entry(idt, 23, (uint32_t)exception23, 0x08, 0x8E);
    set_idt_entry(idt, 24, (uint32_t)exception24, 0x08, 0x8E);
    set_idt_entry(idt, 25, (uint32_t)exception25, 0x08, 0x8E);
    set_idt_entry(idt, 26, (uint32_t)exception26, 0x08, 0x8E);
    set_idt_entry(idt, 27, (uint32_t)exception27, 0x08, 0x8E);
    set_idt_entry(idt, 28, (uint32_t)exception28, 0x08, 0x8E);
    set_idt_entry(idt, 29, (uint32_t)exception29, 0x08, 0x8E);
    set_idt_entry(idt, 30, (uint32_t)exception30, 0x08, 0x8E);
    set_idt_entry(idt, 31, (uint32_t)exception31, 0x08, 0x8E);

    // IRQs
    set_idt_entry(idt, 32, (uint32_t)irq0, 0x08, 0x8E);
    set_idt_entry(idt, 33, (uint32_t)irq1, 0x08, 0x8E);
    set_idt_entry(idt, 34, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 35, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 36, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 37, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 38, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 39, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 40, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 41, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 42, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 43, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 44, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 45, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 46, (uint32_t)isr_stub, 0x08, 0x8E);
    set_idt_entry(idt, 47, (uint32_t)isr_stub, 0x08, 0x8E);
    idt_t idt_ptr;
    idt_ptr.base = (uint32_t)idt;
    idt_ptr.limit = (256*sizeof(idt_entry_t))-1;
    load_idt(&idt_ptr);
    asm volatile("sti");
}