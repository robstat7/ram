/*
 * Program to create a process and destroy it.
 */
#include "../include/ram/pc.h"
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	struct process_control_block *p;
	
	p = create_process();
	if (p == NULL) {
		printf("couldn't create process!\n");
		return 1;	
	}
	
	printf("process created at %p!\n", p);

	destroy_process(p);
	printf("process destroyed!\n");

	return 0;
}
