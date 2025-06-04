static int range_cmp(const void *_r, const void *_s)
{
	const struct range *r = _r;
	const struct range *s = _s;

	/* this could be simply 'return r.start-s.start', but for the types */
	if (r->start == s->start)
		return 0;
	if (r->start < s->start)
		return -1;
	return 1;
}


// Source: line-log.c
// Lines 76-87
