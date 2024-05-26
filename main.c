/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function
 */
#include <efi.h>
#include "fb.h"
#include <stdint.h>

// void (*hello)(void) = (void (*)())0x4016d0;

int main(struct frame_buffer_descriptor frame_buffer, void *xsdp)
{

	// uint32_t SystemVariables	= 0x0000000000110000; // 0x110000 -> System Variables

	// // Clear all memory after the kernel up to 2MiB
	// __asm__("mov edi, %0\n\t"
	// 	"mov ecx, 122880\n\t"			// Clear 960 KiB
	// 	"mov eax, 0\n\t"
	// 	"rep stosq"
	// 	::"m" (SystemVariables):"edi", "ecx", "rax");


	/* initialize terminal output */
	tty_out_init(frame_buffer);

	/* fill terminal background color with white */
	fill_tty_bgcolor();


	volatile uint8_t *ptr = 0x4016d0;

	printk("@{d}\n", *ptr++);
	printk("@{d}\n", *ptr);

	/* initialize gdt */
	init_gdt();

	/* initialize idt */
	init_idt();


//	__asm__ volatile ("mov $60, %eax; mov $0, %edi; syscall "); 
	

	// int b = 5/0;

	// int * a = NULL;
	// 
	// *a = 5;

	

	/* initialize the timer */
	if(timer_init() == 1)
		goto end;
	
	/* enable APIC interrupt controller */
	enable_apic();


	// /* call test timer function */
	// test_timer();


	/* init nvme */
	// if(nvme_init(xsdp) == 1)
	// 	goto end;




	printk("@loaded gdt and idt!\n");
                                                 
                                                 
       // __asm__("int3"
       // 	:::);

        printk("executing hello...\n");
                                                 
       __asm__("call pre_hello"
       	:::);
                                                 
                                                 
        printk("I should not be printed!...\n");



end:
	/* hang here */
	for(;;);

	return 1;
}
