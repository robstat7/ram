/*
 * 'mm.c' contains the functions for managing memory.
 */
#include "../include/ram/pc.h"
#include <stdio.h>
#include <string.h>

int *process_start = (int *)0x2000;
const unsigned short MAX_TEXT_SIZE = 1000; /* 1KB for text */
const unsigned short MAX_DATA_SIZE = 1000; /* 1KB for data */
const unsigned short MAX_HEAP_SIZE = 10000; /* 10KB for heap */
const unsigned short MAX_STACK_SIZE = 1000; /* 1KB for stack */

int load_program(void)
{
	/* example */
	const char *code = "0000: 0000 0001 0022 0055 0f23\n0001: 0B01 0C01 0D22 0e55 0f23";
	int size = strlen(code);

	printf("@@@doing memcpy\n");
	memcpy(process_start, code, size);
	printf("@@@done\n");

	if (*process_start == code[0])
		return 0;
	else
		return 1;
	
	// printf("@@@%c\n", *process_start);
	// return 0;
}
