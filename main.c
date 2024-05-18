/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function
 */
#include <efi.h>
#include "fb.h"

int main(struct frame_buffer_descriptor frame_buffer, void *xsdp)
{	
	/* initialize terminal output */
	tty_out_init(frame_buffer);

	/* fill terminal background color with white */
	fill_tty_bgcolor();

	/* initialize the timer */
	if(timer_init() == 1)
		goto end;

	// /* call test timer function */
	// test_timer();
	//
	
	/* enable APIC interrupt controller */
	enable_apic();


	/* init nvme */
	// if(nvme_init(xsdp) == 1)
	//	goto end;

end:
	/* hang here */
	for(;;) {
	}

	return 1;
}
