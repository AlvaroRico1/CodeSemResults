static int extract_how_many(int *n, const char *spec, size_t *pos)
{
	const char *end_ptr;
	int parsed, accumulated;
	char kind = spec[*pos];

	GIT_ASSERT_ARG(spec[*pos] == '^' || spec[*pos] == '~');

	accumulated = 0;

	do {
		do {
			(*pos)++;
			accumulated++;
		} while (spec[(*pos)] == kind && kind == '~');

		if (git__isdigit(spec[*pos])) {
			if (git__strntol32(&parsed, spec + *pos, strlen(spec + *pos), &end_ptr, 10) < 0)
				return GIT_EINVALIDSPEC;

			accumulated += (parsed - 1);
			*pos = end_ptr - spec;
		}

	} while (spec[(*pos)] == kind && kind == '~');

	*n = accumulated;

	return 0;
}


// Source: revparse.c
// Lines 575-604
