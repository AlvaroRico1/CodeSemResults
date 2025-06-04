static void filter_diffs_for_paths(struct line_log_data *range, int keep_deletions)
{
	int i;
	struct diff_queue_struct outq;
	DIFF_QUEUE_CLEAR(&outq);

	for (i = 0; i < diff_queued_diff.nr; i++) {
		struct diff_filepair *p = diff_queued_diff.queue[i];
		struct line_log_data *rg = NULL;

		if (!DIFF_FILE_VALID(p->two)) {
			if (keep_deletions)
				diff_q(&outq, p);
			else
				diff_free_filepair(p);
			continue;
		}
		for (rg = range; rg; rg = rg->next) {
			if (!strcmp(rg->path, p->two->path))
				break;
		}
		if (rg)
			diff_q(&outq, p);
		else
			diff_free_filepair(p);
	}


// Source: line-log.c
// Lines 793-818
