void add_pathspec_matches_against_index(const struct pathspec *pathspec,
					struct index_state *istate,
					char *seen,
					enum ps_skip_worktree_action sw_action)
{
	int num_unmatched = 0, i;

	/*
	 * Since we are walking the index as if we were walking the directory,
	 * we have to mark the matched pathspec as seen; otherwise we will
	 * mistakenly think that the user gave a pathspec that did not match
	 * anything.
	 */
	for (i = 0; i < pathspec->nr; i++)
		if (!seen[i])
			num_unmatched++;
	if (!num_unmatched)
		return;
	for (i = 0; i < istate->cache_nr; i++) {
		const struct cache_entry *ce = istate->cache[i];
		if (sw_action == PS_IGNORE_SKIP_WORKTREE &&
		    (ce_skip_worktree(ce) || !path_in_sparse_checkout(ce->name, istate)))
			continue;
		ce_path_match(istate, ce, pathspec, seen);
	}


// Source: pathspec.c
// Lines 22-46
