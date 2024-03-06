#include <efi.h>
#include <efilib.h> 

// typedef struct {
// 	uint16_t	limit;
// 	uint64_t	base;
// } __attribute__((packed)) idtr_t;

// extern idtr_t *efi_idtr;

struct interrupt_frame;

//  __attribute__((interrupt)) void interrupt_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void interrupt_handler(struct interrupt_frame* frame)
{
   //  __asm__ volatile ("cli"); // disable interrupt
   //      __asm__ volatile ("lidt %0" : : "m"(*efi_idtr)); // load the efi IDT
   //  __asm__ volatile ("sti"); // set the interrupt flag

   
  
   
	Print(L"@@@yay! handling interrupt!\n");
	
	// int syscall_num;

	/* store syscall number */
	//asm("movl %%eax, %0" : : "r"(syscall_num));
	// Print(L"@@@syscall #%d is not implemented!\n", syscall_num);
}

//  __attribute__((interrupt)) void exception_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void exception_handler(struct interrupt_frame* frame)
{
	Print(L"@@@exception occured!\n");
}
