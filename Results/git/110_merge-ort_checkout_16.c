static int checkout(struct merge_options *opt,
		    struct tree *prev,
		    struct tree *next)
{
	/* Switch the index/working copy from old to new */
	int ret;
	struct tree_desc trees[2];
	struct unpack_trees_options unpack_opts;

	memset(&unpack_opts, 0, sizeof(unpack_opts));
	unpack_opts.head_idx = -1;
	unpack_opts.src_index = opt->repo->index;
	unpack_opts.dst_index = opt->repo->index;

	setup_unpack_trees_porcelain(&unpack_opts, "merge");

	/*
	 * NOTE: if this were just "git checkout" code, we would probably
	 * read or refresh the cache and check for a conflicted index, but
	 * builtin/merge.c or sequencer.c really needs to read the index
	 * and check for conflicted entries before starting merging for a
	 * good user experience (no sense waiting for merges/rebases before
	 * erroring out), so there's no reason to duplicate that work here.
	 */

	/* 2-way merge to the new branch */
	unpack_opts.update = 1;
	unpack_opts.merge = 1;
	unpack_opts.quiet = 0; /* FIXME: sequencer might want quiet? */
	unpack_opts.verbose_update = (opt->verbosity > 2);
	unpack_opts.fn = twoway_merge;
	unpack_opts.preserve_ignored = 0; /* FIXME: !opts->overwrite_ignore */
	parse_tree(prev);
	init_tree_desc(&trees[0], prev->buffer, prev->size);
	parse_tree(next);
	init_tree_desc(&trees[1], next->buffer, next->size);

	ret = unpack_trees(2, trees, &unpack_opts);
	clear_unpack_trees_porcelain(&unpack_opts);
	return ret;
}


// Source: merge-ort.c
// Lines 4034-4074
