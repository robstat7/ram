#include <stdint.h>

struct gdt_entry {
	uint16_t limit_low;  
	uint16_t base_low; 
	uint8_t base_middle; 
	uint8_t access; 
	uint8_t granularity; 
	uint8_t base_high; 
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct tss_entry64 {
	uint16_t limit_low;  
	uint16_t base_low; 
	uint8_t base_middle; 
	uint8_t access; 
	uint8_t granularity; 
	uint8_t base_high; 
	uint32_t base_high2;
	uint32_t zero;
}__attribute__((packed));
typedef struct tss_entry64 tss_entry64_t;

struct tss64 {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint32_t reserved1;
	uint32_t reserved2;
	uint64_t ist[7];
	uint32_t reserved3;
	uint32_t reserved4;
	uint16_t reserved5;
	uint16_t ioMapBase;
}__attribute__((packed));
typedef struct tss64 tss64_t;

struct gdt_ptr {
	uint16_t limit;
	gdt_entry_t *base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

gdt_entry_t gdt_entries[7];
gdt_ptr_t gdt_ptr;

tss64_t tss;

extern void gdt_load(gdt_ptr_t *gdt_ptr);

void init_gdt();
void gdt_set_gate(int, uint32_t, uint32_t, uint8_t, uint8_t);
void gdt_set_tss(tss_entry64_t*, tss64_t*, uint32_t, uint8_t, uint8_t);
