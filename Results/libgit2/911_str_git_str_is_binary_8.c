int git_str_is_binary(const git_str *buf)
{
	const char *scan = buf->ptr, *end = buf->ptr + buf->size;
	git_str_bom_t bom;
	int printable = 0, nonprintable = 0;

	scan += git_str_detect_bom(&bom, buf);

	if (bom > GIT_STR_BOM_UTF8)
		return 1;

	while (scan < end) {
		unsigned char c = *scan++;

		/* Printable characters are those above SPACE (0x1F) excluding DEL,
		 * and including BS, ESC and FF.
		 */
		if ((c > 0x1F && c != 127) || c == '\b' || c == '\033' || c == '\014')
			printable++;
		else if (c == '\0')
			return true;
		else if (!git__isspace(c))
			nonprintable++;
	}

	return ((printable >> 7) < nonprintable);
}


// Source: str.c
// Lines 1241-1267
