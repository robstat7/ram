#include "include/printk.h"
#include <stdbool.h>

char* citoa(int num, char* str);

void print_arg(const char *specifier, va_list *ap)
{
	char ch;
	int arg;
	char str[60];

		switch(specifier[0]) {
			case 'c':
                                ch = va_arg(*ap, int);
                        	write_char(ch);
				break;
			case 'd':
                                arg = va_arg(*ap, int);
				citoa(arg, str);
				printk(str);
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

	state = NORMAL;

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
		case 'd':
			if (state == NORMAL)
				write_char(current);
			else
				specifier = "d";
			break;
		default:
			write_char(current);
			break;
		}
	}

	va_end(ap);
}

// A utility function to reverse a string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

/*
 * convert integer to string.
 */
char* citoa(int num, char* str)
{
    int i = 0;
    bool isNegative = false;
    int base;
 
    base = 10;
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}
