/*
 * Terminal driver.
 */
#include "include/frame_buffer.h"
#include "include/fonts.h"
#include <stdint.h>

struct frame_buffer_descriptor frame_buffer;

static inline void write_pixel_32bpp(uint32_t pixel, int x, int y)
{
        *((uint32_t*)(frame_buffer.frame_buffer_base + frame_buffer.pixels_per_scan_line * y + x)) = pixel;
}
	 
void write_char(unsigned char c, int x, int y, uint32_t fgcolor, uint32_t bgcolor)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;

	offset = (int) c;
	offset = offset * 8;

	for(cy=0;cy<8;cy++){
		for(cx=0;cx<8;cx++){
			write_pixel_32bpp(console_font_8x8[offset+cy]&mask[cx]?fgcolor:bgcolor,x+cx,y+cy);
		}
	}
}	

void tty_init(struct frame_buffer_descriptor fb) {
	frame_buffer = fb;
}
