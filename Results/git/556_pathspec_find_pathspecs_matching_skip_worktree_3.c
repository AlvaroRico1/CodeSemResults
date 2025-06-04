char *find_pathspecs_matching_skip_worktree(const struct pathspec *pathspec)
{
	struct index_state *istate = the_repository->index;
	char *seen = xcalloc(pathspec->nr, 1);
	int i;

	for (i = 0; i < istate->cache_nr; i++) {
		struct cache_entry *ce = istate->cache[i];
		if (ce_skip_worktree(ce) || !path_in_sparse_checkout(ce->name, istate))
		    ce_path_match(istate, ce, pathspec, seen);
	}


// Source: pathspec.c
// Lines 66-76
