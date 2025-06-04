format_trim_left(const char *expanded, u_int limit)
{
	char			*copy, *out;
	const char		*cp = expanded, *end;
	u_int			 n, width = 0, leading_width;
	struct utf8_data	 ud;
	enum utf8_state		 more;

	out = copy = xcalloc(1, strlen(expanded) + 1);
	while (*cp != '\0') {
		if (width >= limit)
			break;
		if (*cp == '#') {
			end = format_leading_hashes(cp, &n, &leading_width);
			if (leading_width > limit - width)
				leading_width = limit - width;
			if (leading_width != 0) {
				if (n == 1)
					*out++ = '#';
				else {
					memset(out, '#', 2 * leading_width);
					out += 2 * leading_width;
				}
				width += leading_width;
			}
			cp = end;
			if (*cp == '#') {
				end = format_skip(cp + 2, "]");
				if (end == NULL)
					break;
				memcpy(out, cp, end + 1 - cp);
				out += (end + 1 - cp);
				cp = end + 1;
			}
		} else if ((more = utf8_open(&ud, *cp)) == UTF8_MORE) {
			while (*++cp != '\0' && more == UTF8_MORE)
				more = utf8_append(&ud, *cp);
			if (more == UTF8_DONE) {
				if (width + ud.width <= limit) {
					memcpy(out, ud.data, ud.size);
					out += ud.size;
				}
				width += ud.width;
			} else {
				cp -= ud.have;
				cp++;
			}
		} else if (*cp > 0x1f && *cp < 0x7f) {
			if (width + 1 <= limit)
				*out++ = *cp;
			width++;
			cp++;
		} else
			cp++;
	}
	*out = '\0';
	return (copy);
}


// Source: format-draw.c
// Lines 1073-1130
