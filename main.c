/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function
 */
#include <efi.h>
#include "printk.h"
#include "fb.h"
#include <stdint.h>
#include "string.h"

int main(struct frame_buffer_descriptor frame_buffer, void *xsdp)
{
	int i;
	uint32_t msg1, msg2, msg3, value1, value2, value3;
	uint16_t cpuid_ax;
	char *gen_intel_msg = "GenuineIntel";

	/* initialize terminal output */
	tty_out_init(frame_buffer);

	/* fill terminal background color with white */
	fill_tty_bgcolor();

	/* initialize timer (x2APIC) */

	/* check for the "GenuineIntel" message */
	__asm__("mov eax, 0\n\t"
		"cpuid\n\t"
		"mov %0, ebx\n\t"
		"mov %1, edx\n\t"
		"mov %2, ecx"
		::"m" (msg1),
		"m" (msg2),
		"m" (msg3):);

	for(i = 0; i < 4; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg1, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg1 >>= 8;
	}

	for(; i < 8; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg2, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg2 >>= 8;
	}

	for(; i < 12; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg3, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg3 >>= 8;
	}


	printk("@timer: cpuid: check for GenIntel message passed!\n");

	__asm__("mov eax, 0x1\n\t"
		"cpuid\n\t"
		"mov eax, 0x10\n\t"
		"mov ebx, 0x200000\n\t"
		"mov esi, edx\n\t"
		"and esi, 0x200\n\t"
		"and edx, eax\n\t"
		"and ecx, ebx\n\t"
		"mov %0, edx\n\t"
		"mov %1, ecx\n\t"
		"mov %2, esi"
		::"m" (value1),
		"m" (value2),
		"m" (value3):);
		
	/* check whether MSRs are supported */
	if (value1 == 0x10) {
		printk("@timer: cpuid: MSRs are supported!\n");
	} else if (value1 == 0x0) {
		printk("error: timer: cpuid: MSRs aren't supported!\n");
		goto end;
	}

	/* check whether the CPU has a built-in local APIC and if it hasn't been disabled in MSRs */
	if(value3 == 0x200) {
		printk("@timer: cpuid: the cpu has a built-in local apic!\n");
	} else {
		printk("error: timer: cpuid: either the cpu doesn't have a local apic or it has been disabled in MSRs!\n");
		goto end;
	}

	/* detecting x2APIC mode */
	if (value2 == 0x200000) {
		printk("@timer: cpuid: the processor supports the x2APIC capability!\n");
	} else if (value2 == 0x0) {
		printk("error: timer: cpuid: the processor doesn't support the x2APIC capability!\n");
		goto end;
	}

	/* enable the local apic */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x1b\n\t"
		"rdmsr\n\t"
		"or eax, 0x800\n\t"
		"wrmsr\n\t"
		"rdmsr\n\t"
		"and eax, 0x800\n\t"
		"mov %0, eax"
		::"m" (value1):);
	
	if (value1 == 0x800) {
		printk("@timer: init: enabled the local APIC!\n");
	} else {
		printk("error: timer: init: unable to enable the local APIC!\n");
		goto end;
	}

	/* enabling x2APIC mode */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x1b\n\t"
		"rdmsr\n\t"
		"or eax, 0x400\n\t"
		"wrmsr\n\t"
		"rdmsr\n\t"
		"and eax, 0xc00\n\t"
		"mov %0, eax"
		::"m" (value1):);

	if (value1 == 0xc00) {
		printk("@timer: init: enabled the local APIC in x2APIC mode!\n");
	} else {
		printk("error: timer: init: unable to enable the local APIC in x2APIC mode!\n");
		goto end;
	}

	/* get the lapic frequency equal to the core crystal's frequency */
	__asm__("mov eax, 0x16\n\t"
		"cpuid\n\t"
		"mov %0, ax"
		::"m" (cpuid_ax):);

	printk("@timer: cpuid: core base frequency is {d} MHz\n", cpuid_ax);


	/* init nvme */
	// if(nvme_init(xsdp) == 1)
	//	goto end;

end:
	/* hang here */
	for(;;) {
	}

	return 1;
}
