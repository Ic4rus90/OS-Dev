#include "../include/pit.h"
#include "../include/common.h"
#include "../include/isr.h"
#include "system.h"

volatile uint32_t tick = 0;

uint32_t get_current_tick() {
    return tick;
}

void pit_irq_handler() {
    tick++;
    outb(PIC1_CMD_PORT, PIC_EOI); // Send EOI (end of interrupt) to PIC
}

void init_pit() {
    uint16_t divisor = DIVIDER;

    // Register IRQ0 handler
    register_interrupt_handler(32, pit_irq_handler);

    // Send command
    outb(PIT_CMD_PORT, 0x36); // Command 0x36 sets repeating mode

    // Divisor has to be sent byte-wise
    uint8_t low_byte = (uint8_t) (divisor & 0xFF);
    uint8_t high_byte = (uint8_t) ((divisor >> 8) & 0xFF);

    // Send frequency divisor
    outb(PIT_CHANNEL0_PORT, low_byte);
    outb(PIT_CHANNEL0_PORT, high_byte);
}

void sleep_interrupt(uint32_t milliseconds) {

    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    while (current_tick < end_ticks) {
        asm volatile("sti\n\t"
                     "hlt\n\t"
                : /* No output */
                : /* No input */
                : /* Clobbers */ "memory");

        current_tick = get_current_tick();
    }

}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        while (get_current_tick() == start_tick + elapsed_ticks) {
            // Busy wait, do nothing
        }
        elapsed_ticks++;
    }
}