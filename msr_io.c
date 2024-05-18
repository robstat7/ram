/*
 * MSR register read and write functions.
 */
#include "msr_io.h"
 
void write_msr_reg(uint32_t reg_address, uint32_t val)
 {
 	__asm__("mov ecx, %0\n\t"
		"mov edx, 0x0\n\t"
 		"mov eax, %1\n\t"
 		"wrmsr"
 		::"m" (reg_address),
 		"m" (val):);
 }
