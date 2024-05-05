/*
 * NVMe PCIe driver.
 */
#include <stdio.h>
#include <string.h>
#include <printk.h>
#include <stdint.h>

#define NVMe_VS		0x8	/* BAR0 Version register's offset */

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
uint64_t* nvme_base = NULL;
int8_t nvme_mjr_num = 0;
uint8_t nvme_mnr_num = 0, nvme_ter_num = 0, nvme_irq = 0;

unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);
uint32_t check_mcfg_checksum(uint64_t *mcfg);
void check_all_buses(uint16_t start, uint16_t end);
int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function);
uint64_t *get_bar0(uint16_t bus, uint8_t device, uint8_t function);
int check_nvme_vs(uint64_t *nvme_base);
uint8_t get_device_irq_num(uint16_t bus, uint8_t device, uint8_t function);
void *get_base_phy_addr(uint16_t bus, uint8_t device, uint8_t function);
void enable_pci_bus_mastering(void);

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
		printk("found nvme controller! bus num={d}, device num={d}, function num={d}\n\n", detected_bus_num, detected_device_num, detected_function_num);
	} else {
		printk("couldn't found the nvme controller!\n");
		return 1;
	}

	/* read register 4 for BAR0 */
	nvme_base = get_bar0(detected_bus_num, detected_device_num, detected_function_num);

	printk("@nvme_base={p}\n", (void *) nvme_base);

	/* check for a valid version number (bits 31:16 should be greater than 0) */
	if(check_nvme_vs(nvme_base) == 1) {
		return 1;
	}

	printk("@nvme version={d}.{d}.{d}\n", nvme_mjr_num, nvme_mnr_num, nvme_ter_num);

	/* Grab the IRQ of the device */
	nvme_irq = get_device_irq_num(detected_bus_num, detected_device_num, detected_function_num);

	printk("@nvme_irq={d}\n", nvme_irq);

	/* enable pcie bus mastering */
	enable_pci_bus_mastering();

	return 0;
}


void enable_pci_bus_mastering(void)
{
	void *phy_addr;
	int32_t value;

	phy_addr = get_base_phy_addr(detected_bus_num, detected_device_num, detected_function_num);

	phy_addr = (void *) ((char *) phy_addr + (1 * 4));	/* get status/command from pcie device's register #1 */

	value = *((int32_t *) phy_addr);

	__asm__("mov eax, %0\n\t"
		"bts eax, 2\n\t"
		"mov %0, eax"
		::"m" (value):);
	
	*((int32_t *) phy_addr) = value;	/* write value to pcie device's register #1 */

}


void *get_base_phy_addr(uint16_t bus, uint8_t device, uint8_t function)
{
	void *phy_addr;

	phy_addr = (void *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	return phy_addr;
}


uint8_t get_device_irq_num(uint16_t bus, uint8_t device, uint8_t function)
{
	void *phy_addr;
	unsigned char value;

	phy_addr = get_base_phy_addr(bus, device, function);

	phy_addr = (void *) ((char *) phy_addr + (15 * 4));	/* device's IRQ number from PCIe Register 15 (IRQ is bits 7-0). Multiplying by 4 because each register consists of 4 bytes */

	value = *((uint8_t *) phy_addr);

	return (uint8_t) value;
}

/* returns 0 on success else 1 on failure */
int check_nvme_vs(uint64_t *nvme_base)
{
	uint32_t *phy_addr;
	uint32_t value;
	int8_t mjr_num = 0, mnr_num, ter_num;

	phy_addr = (uint32_t *) ((char *) nvme_base + NVMe_VS);

	value = *phy_addr;

	__asm__("mov eax, %0\n\t"
		"ror eax, 16\n\t" /* rotate eax so MJR is bits 15:00 */
		"mov %1, al"
		::"m" (value),
		"m" (mjr_num):);

	if (mjr_num < 1) {
		printk("error: nvme: invalid major version number!\n");
		return 1;
	}

	nvme_mjr_num = mjr_num;	/* store major version num */

	__asm__("rol eax, 8\n\t" /* rotate eax so MNR is bits 07:00 */
		"mov %0, al\n\t"
		"rol eax, 8\n\t" /* rotate eax so TER is bits 07:00 */
		"mov %1, al"
		::"m" (mnr_num),
		"m" (ter_num):);

	nvme_mnr_num = mnr_num;	/* store minor and tertiary ver num */
	nvme_ter_num = ter_num;

	return 0;
}

/* read register 4 for BAR0 */
uint64_t *get_bar0(uint16_t bus, uint8_t device, uint8_t function) {
	uint64_t *phy_addr;
	uint32_t value;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = (uint64_t *) ((char *) phy_addr + (4 * 4));	/* Multiplying by 4 because each register consists of 4 bytes */

	value = *((uint32_t *) phy_addr);

	value = value & 0xFFFFFFF0;		/* clear the lowest 4 bits */
	
	// value = value >> 14;

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
