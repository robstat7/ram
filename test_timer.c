/*
 * test the timer driver
 */
#include "printk.h"
#include <stdint.h>

void test_timer(void)
{
	/* set divide value */
	set_divide_value(0xb); /* divide by 1 (111) */

	printk("test: timer: divide value set to 1!\n");

	/* set mode */
	/* 0x0 = one-shot, 0x20000= periodic, 0x40000 = tsc-deadline */
	uint32_t mode = 0x40000;

	if(set_mode(mode) == 0) 
		printk("test: timer: mode set to {p}!\n", (void *) mode);
	else {
		printk("error: test: timer: couldn't set mode to {p}!\n", (void *) mode);
		return;
	}

	/* set initial count */
	uint64_t initial_cnt = 100;

	set_initial_count(initial_cnt);

	printk("test: timer: initial count set to {d}!\n", initial_cnt);

	/* read current count */
	uint32_t count;
	
	for(int i = 0; i < 3; i++) {
		count = read_current_count();
		printk("test: timer: current count = {d}!\n", count);
	}
}
