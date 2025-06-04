void add_pattern(const char *string, const char *base,
		 int baselen, struct pattern_list *pl, int srcpos)
{
	struct path_pattern *pattern;
	int patternlen;
	unsigned flags;
	int nowildcardlen;

	parse_path_pattern(&string, &patternlen, &flags, &nowildcardlen);
	if (flags & PATTERN_FLAG_MUSTBEDIR) {
		FLEXPTR_ALLOC_MEM(pattern, pattern, string, patternlen);
	} else {
		pattern = xmalloc(sizeof(*pattern));
		pattern->pattern = string;
	}
	pattern->patternlen = patternlen;
	pattern->nowildcardlen = nowildcardlen;
	pattern->base = base;
	pattern->baselen = baselen;
	pattern->flags = flags;
	pattern->srcpos = srcpos;
	ALLOC_GROW(pl->patterns, pl->nr + 1, pl->alloc);
	pl->patterns[pl->nr++] = pattern;
	pattern->pl = pl;

	add_pattern_to_hashsets(pl, pattern);
}


// Source: dir.c
// Lines 875-901
