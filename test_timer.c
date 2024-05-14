/*
 * test the timer driver
 */
#include "printk.h"

void test_timer(void)
{
	set_divide_value(11); /* divide by 1 (111) */

	printk("test: timer: divide value set to 1!\n");
}
