static char *joinpath(const char *dir, const char *base, int base_len)
{
	char *out;
	int len;

	if (base_len == -1) {
		size_t bl = strlen(base);

		cl_assert(bl < INT_MAX);
		base_len = (int)bl;
	}


// Source: fs.h
// Lines 345-355
