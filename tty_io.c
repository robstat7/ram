/*
 * Terminal driver.
 */
#include "include/frame_buffer.h"
#include "include/fonts.h"
#include <stdint.h>

struct frame_buffer_descriptor frame_buffer;
const int font_width = 8;
const int font_height = 8;

static inline void write_pixel(uint32_t pixel, int x, int y)
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
			write_pixel(console_font_8x8[offset+cy]&mask[cx]?fgcolor:bgcolor,x+cx,y+cy);
		}
	}
}	

void write_str(char *str, int x, int y, uint32_t fgcolor, uint32_t bgcolor)
{
	int i;
	int pos_x;
	int pos_y;
	int new_line;

	new_line = 0;
	pos_x = x;
	pos_y = y;
	for (i = 0; str[i] != '\0'; i++) {
		write_char(str[i], pos_x, pos_y, fgcolor, bgcolor);
		if ((pos_x + font_width) >= frame_buffer.pixels_per_scan_line) {
			new_line = 1;
			pos_x = (pos_x + font_width) % frame_buffer.pixels_per_scan_line;
		} else {
			pos_x = pos_x + font_width;
		}
		
		if (new_line == 1) {
			pos_y = (pos_y + font_height);
			new_line = 0;
		}
	}
}

void tty_init(struct frame_buffer_descriptor fb) {
	frame_buffer = fb;
}
