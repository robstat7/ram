/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function
 */
#include <efi.h>
#include <printk.h>
#include <fb.h>
#include <stdint.h>
#include <string.h>

int main(struct frame_buffer_descriptor frame_buffer, void *xsdp)
{
	int i;
	uint32_t msg1, msg2, msg3;
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
			printk("timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg1 >>= 8;
	}

	for(; i < 8; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg2, 1) !=0) {
			printk("timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg2 >>= 8;
	}

	for(; i < 12; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg3, 1) !=0) {
			printk("timer: cpuid: processor is not genuine intel!\n");
			goto end;
		}
		msg3 >>= 8;
	}


	printk("@cpuid: check for GenIntel message passed!\n");


	/* init nvme */
	// if(nvme_init(xsdp) == 1)
	//	goto end;

end:
	/* hang here */
	for(;;) {
	}

	return 1;
}
