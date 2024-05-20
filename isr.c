#include "isr.h"
#include <efi.h>
#include <efilib.h> 

isr_t interrupt_handlers[256];

void isr_handler(registers_t regs) {
  // Print(L"recieved interrupt: ");
  //Print(L"I was here %x\n", regs.rbp);
  Print(L"rip=%x\n", regs.rip);
  // Print(L"syscall #%d called!\n", regs.rax);
  Print(L"ISR #%d called!\n", regs.int_no);
  // for(;;) {}
}

void irq_handler(registers_t regs) {
  if (regs.int_no != 0x20)
    Print(L"recieved interrupt: 0x%x\n",regs.int_no);

  if ((regs.int_no == 0x2f) || (regs.int_no == 0x2e)) {
  }
  if (regs.int_no >= 40) {
    // outb(0xA0, 0x20);
  }

  //outb(0x20, 0x20);

  if (interrupt_handlers[regs.int_no] != 0) {
    isr_t handler = interrupt_handlers[regs.int_no];
    handler(regs);
  }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
  interrupt_handlers[n] = handler;
}

