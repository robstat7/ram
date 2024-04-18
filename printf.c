int strlen(const char *str);

void printf(const char *str, char *arg)
{
	int len;
	int i;

	len = strlen(str);

	for(i=0; i<len; i++) {
		if(str[i]=='%' && (i + 1) < len) {
			if(str[i + 1] == '%') {
				i += 2;
			} else if (str[i + 1] == 's') {
				write_str(arg);
				i += 2;
				continue;
			}
		}
		write_char(str[i]);
	}
}
				

int strlen(const char *str)
{
	int i;

	for(i = 0; str[i] != '\0'; i++);

	return i;
}


