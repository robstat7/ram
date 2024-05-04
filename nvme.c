/*
 * NVMe PCIe driver.
 */
#include <stdio.h>
#include <string.h>
#include <printk.h>
#include <stdint.h>

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
uint32_t* nvme_base = NULL;

unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);
uint32_t check_mcfg_checksum(uint64_t *mcfg);
void check_all_buses(uint16_t start, uint16_t end);
int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function);
uint32_t *get_bar0(uint16_t bus, uint8_t device, uint8_t function);

int nvme_init(void *xsdp)
{
	int i;
	uint64_t *xsdt;
	uint32_t xsdt_length;
	int num_entries;
	uint64_t *desc_header;
	uint64_t *mcfg;
	char desc_header_sig[4];
	uint16_t start_bus_num;
	uint16_t end_bus_num;

	mcfg = NULL;

	/* get physical address of the XSDT */
	xsdt = (uint64_t *) *((uint64_t *) ((char *) xsdp + 24));

	xsdt_length = *((uint32_t *) ((unsigned char *) xsdt + 4));

	/* check for valid XSDT checksum */
	if(check_xsdt_checksum(xsdt, xsdt_length) != 0) {
		printk("error: invalid xsdt table!\n");
		return 1;
	}

	num_entries = (xsdt_length - 36)/8 ;

	/* find and store MCFG table pointer */
	for(i = 0; i < num_entries; i++) {
		desc_header = (uint64_t *) ((uint64_t *) ((char *) xsdt + 36))[i];

		strncpy(desc_header_sig, (char *) desc_header, 4);

		/* check MCFG table signature */
		if(strncmp(desc_header_sig, "MCFG", 4) == 0) {
			/* check for valid MCFG checksum */
			if(check_mcfg_checksum(desc_header) == 0) {
				/* This is our MCFG table. Store its pointer */
				mcfg = desc_header;
				break;
			}
		}
	}

	if(mcfg == NULL) {
		printk("error: could not find MCFG table!\n");
		return 1;
	}

	/* get and store ECAM base address and starting and ending pcie bus number */
	pcie_ecam = (uint64_t *) *((uint64_t *) ((char *) mcfg + 44));
	start_bus_num = (uint16_t) *((unsigned char *) mcfg + 44 + 10);
	end_bus_num = (uint16_t) *(((unsigned char *) mcfg) + 44 + 11);

	/* enumerate pcie buses to find the nvme controller */
	check_all_buses(start_bus_num, end_bus_num);

	if(detected_bus_num != -1 && detected_device_num != -1 && detected_function_num != -1) {
		printk("found nvme controller! bus num={d}, device num={d}, function num={d}\n", detected_bus_num, detected_device_num, detected_function_num);
	} else {
		printk("couldn't found the nvme controller!\n");
		return 1;
	}

	/* read register 4 for BAR0 */
	nvme_base = get_bar0(detected_bus_num, detected_device_num, detected_function_num);

	printk("@nvme_base={p}", (void *) nvme_base);
	
	return 0;
}

uint32_t *get_bar0(uint16_t bus, uint8_t device, uint8_t function) {
	uint64_t *phy_addr;
	uint32_t value;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = (uint64_t *) ((char *) phy_addr + 16);

	value = *((uint32_t *) phy_addr);
	
	value = value >> 14;

	return value;
}


int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function) {
	uint64_t *phy_addr;
	uint32_t value;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = (uint64_t *) ((char *) phy_addr + 8);

	value = *((uint32_t *) phy_addr);
	
	value = value >> 8;

	if(value == 0x00010802) /* class code = 0x1, subclass code = 0x8, prog if = 0x2 */
		return 0;
	else
		return 1;
}

uint16_t get_vendor_id(uint16_t bus, uint8_t device, uint8_t function)
{
	uint64_t *phy_addr;
	uint64_t value;
	uint16_t vendor_id;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	vendor_id = *((uint16_t *) phy_addr);

	return vendor_id;
}

int check_device(uint16_t bus, uint8_t device)
{
	uint8_t function = 0;
	uint16_t vendor_id;
	int res;

	vendor_id = get_vendor_id(bus, device, function);
	if (vendor_id == 0xFFFF)	/* device doesn't exist */
		return 1;

	res = find_nvme_controller(bus, device, function);	

	return res;
}

void check_all_buses(uint16_t start, uint16_t end)
{
     uint16_t bus;
     uint8_t device;	
     uint8_t found;

     found = 0;

     for(bus = start; bus <= end; bus++) {
         for(device = 0; device < 32; device++) {
		if(check_device(bus, device) == 0) {
			detected_bus_num = (int16_t) bus;
		 	detected_device_num = (int16_t) device;
			detected_function_num = 0;

			found = 1;
			break;
	 	}
        }
	if(found == 1) {
		 break;
	}
     }
}

/*
 * returns 0 if the checksum is valid.
 */
uint32_t check_mcfg_checksum(uint64_t *mcfg)
{
	uint32_t length;
	uint32_t sum;
	uint32_t i;

	sum = 0;

	length = *((uint32_t *) ((unsigned char *) mcfg + 4));

	for(i = 0; i < length; i++)
		sum += ((unsigned char *) mcfg)[i];

	return sum & 0xff;
}

/*
 * returns 0 if the checksum is valid.
 */
unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length)
{
    unsigned char sum;
    int i;

    sum = 0;

    for(i = 0; i < xsdt_length; i++) {
	sum += ((char *) xsdt)[i];
    }

    return sum;
}
