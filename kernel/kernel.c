/* gcc provides these header files automatically */
#include <stddef.h>
#include <stdint.h>

/* this is the x86-64's VGA textmode buffer. To display text, we write data to this memory location */
volatile uint16_t* vga_buffer = (uint16_t*)0xb8000;
/* by default, the VGA textmode buffer has a size of 80x25 characters */
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

/* we start displaying text in the top-left of the screen (column = 0, row = 0) */
int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0f; /* black background, white foreground */

/*
 * void term_init(void);
 *
 * This function initiates the terminal by clearing
 * it.
 *
 * Entries in the VGA buffer take the binary form
 * bbbbffffcccccccc, where:
 * * b is the background color
 * * f is the foreground color
 * * c is the ASCII character
 */
void term_init(void)
{
	int col;
	int row;

	for (col = 0; col < VGA_COLS; col++) {
		for (row = 0; row < VGA_ROWS; row++) {
			/* we find an index into the buffer for our character */
			const size_t index = (VGA_COLS * row) + col;
			/* set the character to blank (a space character) */
			vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
		}
	}
}

/*
 * void term_putc(char c);
 *
 * This function places a single character onto the
 * screen.
 *
 * * Newline characters should return the column to
 *   0, and increment the row.
 * * Normal characters just get displayed and then
 *   increment the column.
 * * If we get past the last column, we need to reset
 *   the column to 0, and increment the row to get
 *   to a new line.
 * * If we get past the last row, we need to reset
 *   both column and row to 0 in order to loop
 *   back to the top of the screen.
 */
void term_putc(char c)
{
	switch (c)
	{
	case '\n': 		
		{
			term_col = 0;
			term_row++;
			break;
		}

	/* for normal characters */
	default:
		{
			/* like before, calculate the buffer index */
			const size_t index = (VGA_COLS * term_row) + term_col;
			vga_buffer[index] = ((uint16_t)term_color << 8) | c;
			term_col++;
			break;
		}
	}

	/* if we get past the last column */
	if (term_col >= VGA_COLS)
	{
		term_col = 0;
		term_row++;
	}

	/* if we get past the last row */
	if (term_row >= VGA_ROWS)
	{
		term_col = 0;
		term_row = 0;
	}
}

/*
 * void term_print(const char* str);
 *
 * This function prints an entire string onto the
 * screen. The string should contain the 
 * null-terminating character.
 */
void term_print(const char* str)
{
	size_t i;

	for (i = 0; str[i] != '\0'; i ++)
		term_putc(str[i]);
}
 
/*
 * void kernel_main();
 *
 * This is our kernel's main function. Initiate the
 * terminal and display a message to show we got
 * here.
 */
void kernel_main()
{
	term_init();

	/* display a message, probably the name of our kernel :) */
	term_print("Ram\n");
}
