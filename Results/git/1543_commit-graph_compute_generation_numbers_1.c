static void compute_generation_numbers(struct write_commit_graph_context *ctx)
{
	int i;
	struct commit_list *list = NULL;

	if (ctx->report_progress)
		ctx->progress = start_delayed_progress(
					_("Computing commit graph generation numbers"),
					ctx->commits.nr);

	if (!ctx->trust_generation_numbers) {
		for (i = 0; i < ctx->commits.nr; i++) {
			struct commit *c = ctx->commits.list[i];
			repo_parse_commit(ctx->r, c);
			commit_graph_data_at(c)->generation = GENERATION_NUMBER_ZERO;
		}
	}

	for (i = 0; i < ctx->commits.nr; i++) {
		struct commit *c = ctx->commits.list[i];
		timestamp_t corrected_commit_date;

		repo_parse_commit(ctx->r, c);
		corrected_commit_date = commit_graph_data_at(c)->generation;

		display_progress(ctx->progress, i + 1);
		if (corrected_commit_date != GENERATION_NUMBER_ZERO)
			continue;

		commit_list_insert(c, &list);
		while (list) {
			struct commit *current = list->item;
			struct commit_list *parent;
			int all_parents_computed = 1;
			timestamp_t max_corrected_commit_date = 0;

			for (parent = current->parents; parent; parent = parent->next) {
				repo_parse_commit(ctx->r, parent->item);
				corrected_commit_date = commit_graph_data_at(parent->item)->generation;

				if (corrected_commit_date == GENERATION_NUMBER_ZERO) {
					all_parents_computed = 0;
					commit_list_insert(parent->item, &list);
					break;
				}

				if (corrected_commit_date > max_corrected_commit_date)
					max_corrected_commit_date = corrected_commit_date;
			}

			if (all_parents_computed) {
				pop_commit(&list);

				if (current->date && current->date > max_corrected_commit_date)
					max_corrected_commit_date = current->date - 1;
				commit_graph_data_at(current)->generation = max_corrected_commit_date + 1;

				if (commit_graph_data_at(current)->generation - current->date > GENERATION_NUMBER_V2_OFFSET_MAX)
					ctx->num_generation_data_overflows++;
			}
		}
	}
	stop_progress(&ctx->progress);
}

static void trace2_bloom_filter_write_statistics(struct write_commit_graph_context *ctx)
{
	trace2_data_intmax("commit-graph", ctx->r, "filter-computed",
			   ctx->count_bloom_filter_computed);
	trace2_data_intmax("commit-graph", ctx->r, "filter-not-computed",
			   ctx->count_bloom_filter_not_computed);
	trace2_data_intmax("commit-graph", ctx->r, "filter-trunc-empty",
			   ctx->count_bloom_filter_trunc_empty);
	trace2_data_intmax("commit-graph", ctx->r, "filter-trunc-large",
			   ctx->count_bloom_filter_trunc_large);
}


// Source: commit-graph.c
// Lines 1500-1575
