static void read_mailmap_string(struct string_list *map, char *buf)
{
	while (*buf) {
		char *end = strchrnul(buf, '\n');

		if (*end)
			*end++ = '\0';

		read_mailmap_line(map, buf);
		buf = end;
	}


// Source: mailmap.c
// Lines 194-204
