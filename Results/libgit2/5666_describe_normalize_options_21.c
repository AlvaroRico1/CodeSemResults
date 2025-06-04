static int normalize_options(
	git_describe_options *dst,
	const git_describe_options *src)
{
	git_describe_options default_options = GIT_DESCRIBE_OPTIONS_INIT;
	if (!src) src = &default_options;

	*dst = *src;

	if (dst->max_candidates_tags > GIT_DESCRIBE_DEFAULT_MAX_CANDIDATES_TAGS)
		dst->max_candidates_tags = GIT_DESCRIBE_DEFAULT_MAX_CANDIDATES_TAGS;

	return 0;
}


// Source: describe.c
// Lines 633-646
