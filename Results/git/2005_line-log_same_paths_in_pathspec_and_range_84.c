static int same_paths_in_pathspec_and_range(struct pathspec *pathspec,
					    struct line_log_data *range)
{
	int i;
	struct line_log_data *r;

	for (i = 0, r = range; i < pathspec->nr && r; i++, r = r->next)
		if (strcmp(pathspec->items[i].match, r->path))
			return 0;
	if (i < pathspec->nr || r)
		/* different number of pathspec items and ranges */
		return 0;

	return 1;
}


// Source: line-log.c
// Lines 741-755
