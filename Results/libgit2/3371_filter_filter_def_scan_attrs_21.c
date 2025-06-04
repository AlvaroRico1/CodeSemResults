static int filter_def_scan_attrs(
	git_str *attrs, size_t *nattr, size_t *nmatch, const char *attr_str)
{
	const char *start, *scan = attr_str;
	int has_eq;

	*nattr = *nmatch = 0;

	if (!scan)
		return 0;

	while (*scan) {
		while (git__isspace(*scan)) scan++;

		for (start = scan, has_eq = 0; *scan && !git__isspace(*scan); ++scan) {
			if (*scan == '=')
				has_eq = 1;
		}

		if (scan > start) {
			(*nattr)++;
			if (has_eq || *start == '-' || *start == '+' || *start == '!')
				(*nmatch)++;

			if (has_eq)
				git_str_putc(attrs, '=');
			git_str_put(attrs, start, scan - start);
			git_str_putc(attrs, '\0');
		}
	}

	return 0;
}


// Source: filter.c
// Lines 72-104
