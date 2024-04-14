/*
 * NVMe PCIe driver.
 */
#include <efi.h>
#include <efilib.h>

int *os_system_variables = (int *) 0x0000000000110000;	/* 0x110000 -> 0x11FFFF	64K System Variables */

void pcie_main(void)
{
	int val;
	int *os_bus_enabled;

	/* check if PCIe was enabled */
	os_bus_enabled = (int *) (os_system_variables + 0x0303);
	val = (char) *os_bus_enabled; /* 1 if PCI is enabled, 2 if PCIe is enabled */

	Print(L"@@@val = %d\n", val);

	if (val ==  2)
		Print(L"PCIe is enabled!\n");
	else if (val == 1)
		Print(L"PCI is enabled!\n");
}
