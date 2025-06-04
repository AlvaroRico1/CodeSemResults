static int process_all_files(struct line_log_data **range_out,
			     struct rev_info *rev,
			     struct diff_queue_struct *queue,
			     struct line_log_data *range)
{
	int i, changed = 0;

	*range_out = line_log_data_copy(range);

	for (i = 0; i < queue->nr; i++) {
		struct diff_ranges *pairdiff = NULL;
		struct diff_filepair *pair = queue->queue[i];
		if (process_diff_filepair(rev, pair, *range_out, &pairdiff)) {
			/*
			 * Store away the diff for later output.  We
			 * tuck it in the ranges we got as _input_,
			 * since that's the commit that caused the
			 * diff.
			 *
			 * NEEDSWORK not enough when we get around to
			 * doing something interesting with merges;
			 * currently each invocation on a merge parent
			 * trashes the previous one's diff.
			 *
			 * NEEDSWORK tramples over data structures not owned here
			 */
			struct line_log_data *rg = range;
			changed++;
			while (rg && strcmp(rg->path, pair->two->path))
				rg = rg->next;
			assert(rg);
			rg->pair = diff_filepair_dup(queue->queue[i]);
			memcpy(&rg->diff, pairdiff, sizeof(struct diff_ranges));
		}
		free(pairdiff);
	}


// Source: line-log.c
// Lines 1099-1134
