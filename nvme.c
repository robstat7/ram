/*
 * NVMe PCIe driver.
 */
#include <efi.h>
#include <efilib.h>
#include <string.h>
#include <stdint.h>

int nvme_init(EFI_SYSTEM_TABLE *SystemTable)
{
	int i;
	int num_config_tables;
	EFI_CONFIGURATION_TABLE *config_tables;
	EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;	/* EFI GUID for a pointer to the ACPI 2.0 or later specification RSDP structure */
	char *rsdp_struct;
	uint64_t *xsdt_address;
	uint32_t xsdt_length;
	int num_entries;
	uint64_t *desc_header;
	uint64_t *mcfg;
	char desc_header_sig[4];
	uint64_t *ecam_base_addr;
	int start_bus_num;
	int end_bus_num;

	ecam_base_addr = NULL;
	rsdp_struct = NULL;
	mcfg = NULL;

	/* locating and storing the pointer to the RSDP structure */	
	num_config_tables = SystemTable->NumberOfTableEntries;

	config_tables = SystemTable->ConfigurationTable;	

	for(i = 0; i < num_config_tables; i++) {
		if (CompareGuid(&config_tables[i].VendorGuid, &Acpi20TableGuid) == 0) {
			rsdp_struct = (char *) config_tables[i].VendorTable;
			break;
		}
	}

	if(rsdp_struct == NULL) {
		Print(L"error: could not find RSDP structure pointer!\n");
		return 1;
	}

	/* get physical address of the XSDT */
	xsdt_address = (uint64_t *) *((uint64_t *) (rsdp_struct + 24));

	xsdt_length = *((uint32_t *) (((char *) xsdt_address) + 4));

	num_entries = (xsdt_length - 36)/8 ;

	/* find and store MCFG table pointer */
	for(i = 0; i < num_entries; i++) {
		desc_header = (uint64_t *) ((uint64_t *) ((char *) xsdt_address + 36))[i];
		strncpy(desc_header_sig, (char *) desc_header, 4);

		if(strncmp(desc_header_sig, "MCFG", 4) == 0) {
			mcfg = desc_header;
			break;
		}
	}

	if(mcfg == NULL) {
		Print(L"error: could not find MCFG table!\n");
		return 1;
	}


	/* get ECAM base address and starting and ending pcie bus number */
	ecam_base_addr = (uint64_t *) *(mcfg + 44);
	start_bus_num = (int) *((char *) mcfg + 44 + 10);
	end_bus_num = (int) *((char *) mcfg + 44 + 11);

	return 0;
}
