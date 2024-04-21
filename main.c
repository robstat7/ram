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
	int num;

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

	/* printk("{c}", ch); */ 
	/* printk("B{c}", ch); */	
	/* printk("{c}B", ch); */	
	/* printk("c{c}", ch); */	
	/* printk("hello,{c}, how are you?", ch); */
	
	/* printk("Hello world, how is the code going?"); */

	// printk("Hello");
	// printk("World");
	// printk("12345");
	// printk("\n");
	// printk("int a = 10;\n");
	// printk("return;\n");
	//
	
	num = 52;

	printk("{d}", num);
	// printk("B{d}", num);
	// printk("{d}B\n", num);	
	// printk("B{c}{d}12\n", ch, num);
	// printk("hello,{d}, how are you?", num);


	/* hang here */
	for(;;) {
	}
}
