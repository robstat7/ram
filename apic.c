/*
 * APIC (in x2APIC mode) interrupt controller driver
 */
#include "printk.h"
#include "msr_io.h"

void enable_apic(void)
{
	/* set the spurious interrupt vector register bit 8 to start receiving interrupts */
	write_msr_reg(0x80f, 0x100);

	printk("@apic: started receiving interrupts!\n");
}
