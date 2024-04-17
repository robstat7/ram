/*
 * Terminal driver.
 */
#include "include/frame_buffer.h"
#include "include/fonts.h"
#include <stdint.h>

struct frame_buffer_descriptor frame_buffer;
// const int font_width = 8;
// const int font_height = 8;
int tty_x;
int tty_y;
uint32_t tty_fgcolor = 0x000000; /* tty text color */
uint32_t tty_bgcolor = 0xffffff;

static inline void write_pixel(uint32_t pixel, int x, int y)
{
        *((uint32_t*)(frame_buffer.frame_buffer_base + frame_buffer.pixels_per_scan_line * y + x)) = pixel;
}
	 
void write_char(unsigned char c)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;

	offset = (int) c;
	offset = offset * 8;

	for(cy=0;cy<8;cy++){
		for(cx=0;cx<8;cx++){
			write_pixel(console_font_8x8[offset+cy]&mask[cx] ? tty_fgcolor : tty_bgcolor, tty_x + cx, tty_y + cy);
		}
	}

	/* update tty output coords */
	if((tty_x + cx) >= 626) { // 615 old
		tty_x = 11;
		tty_y += cy + 2;
	} else {
		tty_x = tty_x + cx;
	}
}	

void write_str(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; i++) {
		write_char(str[i]);

		// if((tty_x + font_width) >= 626) { // 615 old
		// 	tty_x = 11;
		// 	tty_y += font_height + 2;
		// } else {
		// 	tty_x = tty_x + font_width;
		// }
	}
}

void tty_out_init(struct frame_buffer_descriptor fb) {
	frame_buffer = fb;

	/* init terminal output coordinates */
	tty_x = 11;
	tty_y = 11;
}
