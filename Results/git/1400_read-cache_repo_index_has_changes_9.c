int repo_index_has_changes(struct repository *repo,
			   struct tree *tree,
			   struct strbuf *sb)
{
	struct index_state *istate = repo->index;
	struct object_id cmp;
	int i;

	if (tree)
		cmp = tree->object.oid;
	if (tree || !get_oid_tree("HEAD", &cmp)) {
		struct diff_options opt;

		repo_diff_setup(repo, &opt);
		opt.flags.exit_with_status = 1;
		if (!sb)
			opt.flags.quick = 1;
		diff_setup_done(&opt);
		do_diff_cache(&cmp, &opt);
		diffcore_std(&opt);
		for (i = 0; sb && i < diff_queued_diff.nr; i++) {
			if (i)
				strbuf_addch(sb, ' ');
			strbuf_addstr(sb, diff_queued_diff.queue[i]->two->path);
		}


// Source: read-cache.c
// Lines 2513-2537
