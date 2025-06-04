static int ref_match(const struct strvec *prefixes, const char *refname)
{
	int i;

	if (!prefixes->nr)
		return 1; /* no restriction */

	for (i = 0; i < prefixes->nr; i++) {
		const char *prefix = prefixes->v[i];

		if (starts_with(refname, prefix))
			return 1;
	}


// Source: ls-refs.c
// Lines 53-65
