strnunvis(char *dst, const char *src, size_t sz)
{
	char c, p;
	char *start = dst, *end = dst + sz - 1;
	int state = 0;

	if (sz > 0)
		*end = '\0';
	while ((c = *src++)) {
	again:
		switch (unvis(&p, c, &state, 0)) {
		case UNVIS_VALID:
			if (dst < end)
				*dst = p;
			dst++;
			break;
		case UNVIS_VALIDPUSH:
			if (dst < end)
				*dst = p;
			dst++;
			goto again;
		case 0:
		case UNVIS_NOCHAR:
			break;
		default:
			if (dst <= end)
				*dst = '\0';
			return (-1);
		}
	}
	if (unvis(&p, c, &state, UNVIS_END) == UNVIS_VALID) {
		if (dst < end)
			*dst = p;
		dst++;
	}
	if (dst <= end)
		*dst = '\0';
	return (dst - start);
}


// Source: unvis.c
// Lines 242-280
