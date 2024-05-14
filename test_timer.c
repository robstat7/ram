/*
 * test the timer driver
 */
#include "printk.h"

void test_timer(void)
{
	set_divide_value(0xb); /* divide by 1 (111) */

	printk("test: timer: divide value set to 1!\n");

	int mode = 0x0;

	if(set_mode(0x0) == 0) /* 0x0 = one-shot, 0x20000= periodic, 0x40000 = tsc_deadline */
		printk("test: timer: mode set to {p}!\n", (void *) mode);
	else
		printk("error: test: timer: couldn't set mode to {p}!\n", (void *) mode);
}
