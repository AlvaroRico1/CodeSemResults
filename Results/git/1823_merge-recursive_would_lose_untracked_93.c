static int would_lose_untracked(struct merge_options *opt, const char *path)
{
	struct index_state *istate = opt->repo->index;

	/*
	 * This may look like it can be simplified to:
	 *   return !was_tracked(opt, path) && file_exists(path)
	 * but it can't.  This function needs to know whether path was in
	 * the working tree due to EITHER having been tracked in the index
	 * before the merge OR having been put into the working copy and
	 * index by unpack_trees().  Due to that either-or requirement, we
	 * check the current index instead of the original one.
	 *
	 * Note that we do not need to worry about merge-recursive itself
	 * updating the index after unpack_trees() and before calling this
	 * function, because we strictly require all code paths in
	 * merge-recursive to update the working tree first and the index
	 * second.  Doing otherwise would break
	 * update_file()/would_lose_untracked(); see every comment in this
	 * file which mentions "update_stages".
	 */
	int pos = index_name_pos(istate, path, strlen(path));

	if (pos < 0)
		pos = -1 - pos;
	while (pos < istate->cache_nr &&
	       !strcmp(path, istate->cache[pos]->name)) {
		/*
		 * If stage #0, it is definitely tracked.
		 * If it has stage #2 then it was tracked
		 * before this merge started.  All other
		 * cases the path was not tracked.
		 */
		switch (ce_stage(istate->cache[pos])) {
		case 0:
		case 2:
			return 0;
		}
		pos++;
	}
	return file_exists(path);
}


// Source: merge-recursive.c
// Lines 813-854
