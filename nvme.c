/*
 * NVMe PCIe driver.
 */
#include <stdio.h>
void main()
{
	int val;
	__asm__ __volatile__ ("in %1, %0" : "=a" (val) : "dN" (0x0CFC));
	printf("@@@val=%d\n", val);
}
