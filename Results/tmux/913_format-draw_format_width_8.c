format_width(const char *expanded)
{
	const char		*cp, *end;
	u_int			 n, leading_width, width = 0;
	struct utf8_data	 ud;
	enum utf8_state		 more;

	cp = expanded;
	while (*cp != '\0') {
		if (*cp == '#') {
			end = format_leading_hashes(cp, &n, &leading_width);
			width += leading_width;
			cp = end;
			if (*cp == '#') {
				end = format_skip(cp + 2, "]");
				if (end == NULL)
					return (0);
				cp = end + 1;
			}
		} else if ((more = utf8_open(&ud, *cp)) == UTF8_MORE) {
			while (*++cp != '\0' && more == UTF8_MORE)
				more = utf8_append(&ud, *cp);
			if (more == UTF8_DONE)
				width += ud.width;
			else
				cp -= ud.have;
		} else if (*cp > 0x1f && *cp < 0x7f) {
			width++;
			cp++;
		} else
			cp++;
	}
	return (width);
}


// Source: format-draw.c
// Lines 1032-1065
