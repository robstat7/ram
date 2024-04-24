/*
 * NVMe PCIe driver.
 */
#include <string.h>
#include <printk.h>
#include <stdint.h>

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;

void check_all_buses(uint8_t start, uint8_t end);

int nvme_init(char * rsdp)
{
	int i;
	uint64_t *xsdt_address;
	uint32_t xsdt_length;
	int num_entries;
	uint64_t *desc_header;
	uint64_t *mcfg;
	char desc_header_sig[4];
	uint8_t start_bus_num;
	uint8_t end_bus_num;

	mcfg = NULL;

	

	/* get physical address of the XSDT */
	xsdt_address = (uint64_t *) *((uint64_t *) (rsdp + 24));

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
		printk("error: could not find MCFG table!\n");
		return 1;
	}

	/* get and store ECAM base address and starting and ending pcie bus number */
	pcie_ecam = (uint64_t *) *((uint64_t *) ((char *) mcfg + 44));
	start_bus_num = (uint8_t) *((char *) mcfg + 44 + 10);
	end_bus_num = (uint8_t) *((char *) mcfg + 44 + 11);

	/* enumerate pcie buses */
	check_all_buses(start_bus_num, end_bus_num);

	if(detected_bus_num != -1 && detected_device_num != -1)
		printk("found nvme controller. bus num={d}, device num={d}\n", detected_bus_num, detected_device_num);
	else
		printk("could not found the nvme controller!\n");
	
	printk("@@@pcie scan complete!\n");

	return 0;
}

int find_nvme_controller(uint8_t bus, uint8_t start, uint8_t device, uint8_t function) {
	uint64_t *phy_addr;
	uint32_t value;

	phy_addr = pcie_ecam + ((bus - start) << 20 | device << 15 | function << 12);

	phy_addr = (uint64_t *) ((char *) phy_addr + 8);

	value = *((uint32_t *) phy_addr);

	value = value >> 16;

	if(value == 0x0108) /* class code = 0x1, subclass code = 0x8 */
		return 0;
	else
		return 1;
}


int checkFunction(uint8_t bus, uint8_t start, uint8_t device, uint8_t function) {
	if(find_nvme_controller(bus, start, device, function) == 0) {
		return 0;
	} else {
		return 1;
	}
 }

uint16_t get_vendor_id(uint8_t bus, uint8_t start, uint8_t device, uint8_t function)
{
	uint64_t *phy_addr;
	uint64_t value;
	uint16_t vendor_id;

	phy_addr = pcie_ecam + ((bus - start) << 20 | device << 15 | function << 12);

	value = *phy_addr;

	vendor_id = (uint16_t) value;
	
	return vendor_id;
}

int check_device(uint8_t bus, uint8_t start, uint8_t device)
{
	uint8_t function = 0;
	uint16_t vendor_id;
	int res;

	vendor_id = get_vendor_id(bus, start, device, function);
	if (vendor_id == 0xFFFF)	/* device doesn't exist */
		return 1;

	res = checkFunction(bus, start, device, function);	

	return res;
}

void check_all_buses(uint8_t start, uint8_t end)
{
     uint8_t bus;
     uint8_t device;	
     int found;

     found = 0;
     /* note: not checking the bus no. 125 dev no. 24 and the end bus num devs
      * as they are hanging on reading from the configuration space... */
     for(bus = start; bus < end; bus++) {
         for(device = 0; device < 32; device++) {
		if(bus == 125 && device == 24)
			continue;
		else if(check_device(bus, start, device) == 0) {
			detected_bus_num = (int16_t) bus;
		 	detected_device_num = (int16_t) device;

			found = 1;
			break;
	 	}
        }
	if(found == 1) {
		 break;
	}
     }
}
