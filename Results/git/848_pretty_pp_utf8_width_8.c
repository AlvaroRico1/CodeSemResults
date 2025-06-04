static int pp_utf8_width(const char *start, const char *end)
{
	int width = 0;
	size_t remain = end - start;

	while (remain) {
		int n = utf8_width(&start, &remain);
		if (n < 0 || !start)
			return -1;
		width += n;
	}
	return width;
}


// Source: pretty.c
// Lines 1928-1940
