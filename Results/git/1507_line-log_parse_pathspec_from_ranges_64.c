static void parse_pathspec_from_ranges(struct pathspec *pathspec,
				       struct line_log_data *range)
{
	struct line_log_data *r;
	struct strvec array = STRVEC_INIT;
	const char **paths;

	for (r = range; r; r = r->next)
		strvec_push(&array, r->path);
	paths = strvec_detach(&array);

	parse_pathspec(pathspec, 0, PATHSPEC_PREFER_FULL, "", paths);
	/* strings are now owned by pathspec */
	free(paths);
}


// Source: line-log.c
// Lines 757-771
