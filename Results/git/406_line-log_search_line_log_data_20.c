search_line_log_data(struct line_log_data *list, const char *path,
		     struct line_log_data **insertion_point)
{
	struct line_log_data *p = list;
	if (insertion_point)
		*insertion_point = NULL;
	while (p) {
		int cmp = strcmp(p->path, path);
		if (!cmp)
			return p;
		if (insertion_point && cmp < 0)
			*insertion_point = p;
		p = p->next;
	}
	return NULL;
}


// Source: line-log.c
// Lines 265-280
