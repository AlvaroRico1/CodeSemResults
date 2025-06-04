screen_write_strlen(const char *fmt, ...)
{
	va_list			ap;
	char   	       	       *msg;
	struct utf8_data	ud;
	u_char 	      	       *ptr;
	size_t			left, size = 0;
	enum utf8_state		more;

	va_start(ap, fmt);
	xvasprintf(&msg, fmt, ap);
	va_end(ap);

	ptr = msg;
	while (*ptr != '\0') {
		if (*ptr > 0x7f && utf8_open(&ud, *ptr) == UTF8_MORE) {
			ptr++;

			left = strlen(ptr);
			if (left < (size_t)ud.size - 1)
				break;
			while ((more = utf8_append(&ud, *ptr)) == UTF8_MORE)
				ptr++;
			ptr++;

			if (more == UTF8_DONE)
				size += ud.width;
		} else {
			if (*ptr > 0x1f && *ptr < 0x7f)
				size++;
			ptr++;
		}
	}

	free(msg);
	return (size);
}


// Source: screen-write.c
// Lines 342-378
