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
	char ch;

	/* initialize virtual terminal driver */
	tty_out_init(frame_buffer);

	/* fill terminal background color with white */
	fill_tty_bgcolor();
	

	/* test: write characters onto the terminal */
	// for(i = 0; i < 200; i++) {
	// 	write_char('A');
	// 	write_char('B');
	// }

	// write_char('\n');
	// write_char('p');


	/* test printk */	
	ch = 'A';

	printk("{c}", ch);	
	printk("B{c}", ch);	
	printk("{c}B", ch);	
	printk("c{c}", ch);	

	/* hang here */
	for(;;) {
	}
}
