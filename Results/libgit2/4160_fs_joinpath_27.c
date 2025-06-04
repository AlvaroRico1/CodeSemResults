static char *joinpath(const char *dir, const char *base, int base_len)
{
	char *out;
	int len;

	if (base_len == -1) {
		size_t bl = strlen(base);

		cl_assert(bl < INT_MAX);
		base_len = (int)bl;
	}

	len = strlen(dir) + base_len + 2;
	cl_assert(len > 0);

	cl_assert(out = malloc(len));
	cl_assert(snprintf(out, len, "%s/%.*s", dir, base_len, base) < len);

	return out;
}


// Source: fs.h
// Lines 345-364
