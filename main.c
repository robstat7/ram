/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel main.
 */
#include <efi.h>
#include <printk.h>
#include "include/frame_buffer.h"

void main(struct frame_buffer_descriptor frame_buffer)
{
	/* initialize terminal output */
	tty_out_init(frame_buffer);

	/* fill terminal background color with white */
	fill_tty_bgcolor();
	

	/* hang here */
	for(;;) {
	}
}
