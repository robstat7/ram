#include "include/printk.h"

void print_arg(const char *specifier, va_list *ap)
{
	char ch;

		switch(specifier[0]) {
			case 'c':
                                ch = va_arg(*ap, int);
                        	write_char(ch);
				break;
		}
}


void printk(const char *format, ...)
{
	va_list ap;
	int i;
	int state;
	char current;
	char *specifier;

	va_start(ap, format);		

	for(i = 0; format[i] != '\0'; i++) {
		current = format[i];
 
		switch(current) {
		case '{':
			state = FORMAT_SPECIFIER;
			break;
		case '}':
			print_arg(specifier, &ap);
			state = NORMAL;
			break;
		case 'c':
			if (state == NORMAL)
				write_char(current);
			else
				specifier = "c";
			break;
		default:
			write_char(current);
			break;
		}
	}

	va_end(ap);
}
