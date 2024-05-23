/*
 * NVMe PCIe driver.
 */
#include <stdio.h>
#include "string.h"
#include "printk.h"
#include <stdint.h>

#define NVMe_VS		0x8	/* BAR0 Version register's offset */

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
uint64_t* nvme_base = NULL;
uint8_t nvme_mjr_num = 0;
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
void disable_controller_interrupts(void);

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

	/* disable the controller if it's enabled */
	disable_nvme_controller();	


	/* configure AQA, ASQ, and ACQ */
	config_admin_queues();


	/* disable controller interrupts */
	disable_controller_interrupts();	


	// enable the controller
	enable_nvme_controller();	


	if(nvme_init_enable_wait() == 1) {
		printk("nvme: fatal error! CSTS.CFS (1) is not 0!\n");
		return 1;
	}



	// /* continue initialization in assembly code */
	// __asm__("mov rsi, %0\n\t"
	// 	"call nvme_init_final"
	// 	::"m" (nvme_base):);

	// printk("nvme: controller is initialized!\n");

	// char data[512];
	// /* test: read a text file and it should print- Hello World! */
	// __asm__("mov rax, 421914624\n\t"
	// 	"mov rbx, 2\n\t"
	// 	"mov rcx, 1\n\t"
	// 	"mov rdx, 0\n\t"
	// 	"mov rdi, %0\n\t"
	// 	"call nvme_io"
	// 	::"m" (data):);

	// printk("nvme: done reading data! Output=\n");

	// for (i = 0; i < 13; i++)
	// 	printk("{c}", data[i]);

	// printk("\n");

	printk("done\n");

	return 0;
}

int nvme_init_enable_wait(void)
{
	int nvme_csts =  0x1C; // 4-byte controller status property
	void* addr = (void *) ((uint64_t) nvme_base + nvme_csts);
	uint32_t val;

	do{
		val = *((uint32_t *) addr);

		if((val & 0x2)!= 0x0)	// CSTS.CFS (1) should be 0. If not the controller has had a fatal error
		{
			return 1;
		}
	}while((val & 0x1) != 0x1);	// Wait for CSTS.RDY (0) to become '1'

	return 0;
}

void enable_nvme_controller(void)
{
	uint32_t val = 0x00460001;		// set iocqes (23:20), iosqes (19:16), and en (0)
	int nvme_cc = 0x14;	// 4-byte controller configuration property
	void* addr = (void *) ((uint64_t) nvme_base + nvme_cc);

	*((uint32_t *) addr) = val;	// write the new cc value and enable controller
}

void disable_controller_interrupts(void)
{
	uint32_t val = 0xffffffff;		// mask all interrupts
	int nvme_intms = 0x0C; // 4-byte interrupt mask set
	void* addr_intms = (void *) ((uint64_t) nvme_base + nvme_intms);

	*((uint32_t *) addr_intms) = val;
}

/* 
 * configure AQA, ASQ, and ACQ
 */
void config_admin_queues(void)
{
	int nvme_aqa = 0x24;		// 4-byte Admin Queue Attributes
	void* addr_aqa = (void *) ((uint64_t) nvme_base + nvme_aqa);
	uint32_t value = 0x003f003f;		// 64 commands each for ACQS (27:16) and ASQS (11:00)

	int nvme_asq = 0x28;	// 8-byte admin submission queue base address
	uint64_t nvme_asqb = 0x0000000000170000; // 0x170000 -> 0x170FFF	4K admin submission queue base address
	void* addr_asq = (void *) ((uint64_t) nvme_base + nvme_asq);

	int nvme_acq = 0x30; // 8-byte admin completion queue base address
	uint64_t nvme_acqb = 0x0000000000171000; // 0x171000 -> 0x171FFF	4K admin completion queue base address
	void* addr_acq = (void *) ((uint64_t) nvme_base + nvme_acq);

	// printk("@value={d}\n", value);

	*((uint32_t *) addr_aqa) = value;

	// printk("@value={d}\n", *((uint32_t *) addr_aqa));


	*((uint64_t *) addr_asq) = nvme_asqb;	// ASQB 4K aligned (63:12)

	// printk("@ASQ={llu}\n", *((uint64_t *) addr_asq));
	

	*((uint64_t *) addr_acq) = nvme_acqb;	// ACQB 4K aligned (63:12)
						//
	// printk("@ACQ={llu}\n", *((uint64_t *) addr_acq));
}

void disable_nvme_controller(void)
{
	int nvme_cc = 0x14;	// 4-byte controller configuration property
	void* addr = (void *) ((uint64_t) nvme_base + nvme_cc);
	uint32_t value;

	value = *((uint32_t *) addr);

	// printk("@value={d}\n", value);

	if((value & 0x1) != 0x0) {		// clear CC.EN bit 0 to '0'
		value = (value & 0xfffffffe);
		*((uint32_t *) addr) = value;
	}


	// printk("@value={d}\n", *((uint32_t *) addr));
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
	void *phy_addr;
	uint32_t value, tmp;
	uint8_t mjr_num = 0, mnr_num, ter_num;

	phy_addr = (void *) ((char *) nvme_base + NVMe_VS);

	value = *((uint32_t *) phy_addr);

	__asm__("mov eax, %0\n\t"
		"ror eax, 16\n\t" /* rotate eax so MJR is bits 15:00 */
		"mov %1, al\n\t"
		"mov %2, eax"	  /* copy eax to tmp C variable */
		::"m" (value),
		"m" (mjr_num),
		"m" (tmp):);

	if (mjr_num < 1) {
		printk("error: nvme: invalid major version number!\n");
		return 1;
	}

	nvme_mjr_num = mjr_num;	/* store major version num */

	__asm__("mov eax, %0\n\t" /* copy tmp c variable value to eax */
		"rol eax, 8\n\t" /* rotate eax so MNR is bits 07:00 */
		"mov %1, al\n\t"
		"rol eax, 8\n\t" /* rotate eax so TER is bits 07:00 */
		"mov %2, al"
		::"m" (tmp),
		"m" (mnr_num),
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

	value = (value & 0xFFFFFFF0);		/* clear the lowest 4 bits */
	
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

	return (sum & 0xff);
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
