int repo_is_descendant_of(struct repository *r,
			  struct commit *commit,
			  struct commit_list *with_commit)
{
	if (!with_commit)
		return 1;

	if (generation_numbers_enabled(the_repository)) {
		struct commit_list *from_list = NULL;
		int result;
		commit_list_insert(commit, &from_list);
		result = can_all_from_reach(from_list, with_commit, 0);
		free_commit_list(from_list);
		return result;
	} else {
		while (with_commit) {
			struct commit *other;

			other = with_commit->item;
			with_commit = with_commit->next;
			if (repo_in_merge_bases_many(r, other, 1, &commit))
				return 1;
		}
		return 0;
	}


// Source: commit-reach.c
// Lines 444-468
