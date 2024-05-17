#include <efi.h>
#include <efilib.h> 
#include <string.h>
#include <stdio.h>
#include "descriptor.h"

UINT64 FileSize(EFI_FILE_HANDLE FileHandle);

// typedef struct {
// 	uint16_t	limit;
// 	uint64_t	base;
// } __attribute__((packed)) idtr_t;
// 
// idtr_t *efi_idtr;
//


void init_gdt(void);
void init_idt(void);

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_LOADED_IMAGE *loaded_image;                  /* image interface */
  	EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
  	EFI_FILE_HANDLE Volume;   
	CHAR16              *FileName;
  	EFI_FILE_HANDLE     FileHandle;
	UINT64              ReadSize;
  	UINT8               *Buffer;

	loaded_image = NULL;
	FileName = L"bash";

  	InitializeLib(ImageHandle, SystemTable);

	/* set up the GDT */
	// gdt_install();	
	//
	
	
	/* load bash program 4 loadable segments in memory */
	/* 1. read file into memory first */
	uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &lipGuid, (void **) &loaded_image);
	/* get the volume handle */
	Volume = LibOpenRoot(loaded_image->DeviceHandle);

	/* open the file */
  	uefi_call_wrapper(Volume->Open, 5, Volume, &FileHandle, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	
	/* read file into memory */
	ReadSize = FileSize(FileHandle);
	Buffer = AllocatePool(ReadSize);

	uefi_call_wrapper(FileHandle->Read, 3, FileHandle, &ReadSize, Buffer);

	Print(L"loading segs...\n");
	/* load loadable segments */
	memcpy((void *)0x0000000000400000, (void *)Buffer, (size_t)0x0000000000000720);	
	memcpy((void *)0x0000000000401000, (void *)(Buffer + 0x0000000000001000), (size_t)0x000000000013e9a1);	
	memcpy((void *)0x0000000000540000, (void *)(Buffer + 0x0000000000140000), (size_t)0x0000000000045e1f);	
	memcpy((void *)0x0000000000586978, (void *)(Buffer + 0x0000000000186978), (size_t)0x0000000000008f60);	
	
	

	init_gdt();
	/* init idt */
	init_idt();

	Print(L"@loaded gdt and idt!\n");


	// /* store idt register to variable efi_idtr */
	// // asm volatile ("sidt %0" : "=m"(*efi_idtr));
	
	
	// int a = 5/0;

	// Print(L"done!\n");

	// Print(L"making syscall #38 ...\n");
        // __asm__ volatile ("movq $38, %rax"); 
        // __asm__ volatile ("syscall");

	/* entry point of bash */

	void (*bash)(void) = (void (*)())0x4033e0;
	Print(L"executing bash ...\n");
	bash();

	Print(L"I should not be printed!...\n");

  	FreePool(loaded_image);
  	FreePool(FileName);
  	FreePool(Buffer);

	/* hang here */
	while(1)
	{}

  	return EFI_OUT_OF_RESOURCES;
}

UINT64 FileSize(EFI_FILE_HANDLE FileHandle)
{
  UINT64 ret;
  EFI_FILE_INFO       *FileInfo;         /* file information structure */
  /* get the file's size */
  FileInfo = LibFileInfo(FileHandle);
  ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return ret;
}

void init_gdt() {

  gdt_ptr.limit = (sizeof(gdt_entry_t) * 7) - 1;
  gdt_ptr.base = gdt_entries;

  gdt_set_gate(0,0,0,0,0);
  gdt_set_gate(1,0,0,0x98,0x20);
  gdt_set_gate(2,0,0,0x92,0x20);
  gdt_set_gate(3,0,0,0xF8,0x20);
  gdt_set_gate(4,0,0,0xF2,0x20);

  gdt_set_tss((tss_entry64_t *)&gdt_entries[5], &tss, sizeof(tss),0x89,0);
  gdt_load(&gdt_ptr);

}

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {

  gdt_entries[num].base_low = (base & 0xFFFF);
  gdt_entries[num].base_middle = ((base >> 16) & 0xFF);
  gdt_entries[num].base_high = ((base >> 24) & 0xFF);

  gdt_entries[num].limit_low = (limit & 0xFFFF);
  gdt_entries[num].granularity = (limit >> 16) & 0x0F;

  gdt_entries[num].granularity |= gran & 0xF0;
  gdt_entries[num].access = access;

}

void gdt_set_tss(tss_entry64_t* tss_entry, tss64_t* tss, uint32_t limit, uint8_t access, uint8_t gran) {
  tss_entry->base_low = ((uint64_t)tss & 0xFFFF);
  tss_entry->base_middle = (((uint64_t)tss >> 16) & 0xFF);
  tss_entry->base_high = (((uint64_t)tss >> 24) & 0xFF);

  tss_entry->limit_low = (limit & 0xFFFF);
  tss_entry->granularity = (limit >> 16) & 0x0F;

  tss_entry->granularity |= gran & 0xF0;
  tss_entry->access = access;

  tss_entry->base_high2 = (((uint64_t)tss >> 32) & 0xFFFFFFFF);
  tss_entry->zero = 0;
}

void init_idt() {
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = idt_entries;

  idt_set_gate(0, (uint64_t)isr0, 0x08, 0x8E);
  idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
  idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
  idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
  idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
  idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
  idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
  idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
  idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
  idt_set_gate(9, (uint64_t)isr9, 0x08, 0x8E);
  idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
  idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
  idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
  idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
  idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
  idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
  idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
  idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
  idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
  idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
  idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
  idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);
  idt_set_gate(22, (uint64_t)isr22, 0x08, 0x8E);
  idt_set_gate(23, (uint64_t)isr23, 0x08, 0x8E);
  idt_set_gate(24, (uint64_t)isr24, 0x08, 0x8E);
  idt_set_gate(25, (uint64_t)isr25, 0x08, 0x8E);
  idt_set_gate(26, (uint64_t)isr26, 0x08, 0x8E);
  idt_set_gate(27, (uint64_t)isr27, 0x08, 0x8E);
  idt_set_gate(28, (uint64_t)isr28, 0x08, 0x8E);
  idt_set_gate(29, (uint64_t)isr29, 0x08, 0x8E);
  idt_set_gate(30, (uint64_t)isr30, 0x08, 0x8E);
  idt_set_gate(31, (uint64_t)isr31, 0x08, 0x8E);

  idt_load(&idt_ptr);

  // outb(0x20, 0x11);
  // outb(0xA0, 0x11);
  // outb(0x21, 0x20);
  // outb(0xA1, 0x28);
  // outb(0x21, 0x04);
  // outb(0xA1, 0x02);
  // outb(0x21, 0x01);
  // outb(0xA1, 0x01);
  // outb(0x21, 0x0);
  // outb(0xA1, 0x0);

  idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);
  idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);
  idt_set_gate(34, (uint64_t)irq2, 0x08, 0x8E);
  idt_set_gate(35, (uint64_t)irq3, 0x08, 0x8E);
  idt_set_gate(36, (uint64_t)irq4, 0x08, 0x8E);
  idt_set_gate(37, (uint64_t)irq5, 0x08, 0x8E);
  idt_set_gate(38, (uint64_t)irq6, 0x08, 0x8E);
  idt_set_gate(39, (uint64_t)irq7, 0x08, 0x8E);
  idt_set_gate(40, (uint64_t)irq8, 0x08, 0x8E);
  idt_set_gate(41, (uint64_t)irq9, 0x08, 0x8E);
  idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
  idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
  idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
  idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
  idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
  idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);
}


void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags) {
  idt_entries[num].base_low = base & 0xFFFF;
  idt_entries[num].base_middle = (base>>16) & 0xFFFF;
  idt_entries[num].base_high = (base>>32) & 0xFFFFFFFF;

  idt_entries[num].sel = sel;
  idt_entries[num].always0 = 0;
  idt_entries[num].flags = flags;
  idt_entries[num].zero = 0;
}
