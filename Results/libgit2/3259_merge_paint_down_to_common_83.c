static int paint_down_to_common(
		git_commit_list **out,
		git_revwalk *walk,
		git_commit_list_node *one,
		git_vector *twos,
		uint32_t minimum_generation)
{
	git_pqueue list;
	git_commit_list *result = NULL;
	git_commit_list_node *two;

	int error;
	unsigned int i;

	if (git_pqueue_init(&list, 0, twos->length * 2, git_commit_list_generation_cmp) < 0)
		return -1;

	one->flags |= PARENT1;
	if (git_pqueue_insert(&list, one) < 0)
		return -1;

	git_vector_foreach(twos, i, two) {
		if (git_commit_list_parse(walk, two) < 0)
			return -1;

		two->flags |= PARENT2;
		if (git_pqueue_insert(&list, two) < 0)
			return -1;
	}

	/* as long as there are non-STALE commits */
	while (interesting(&list)) {
		git_commit_list_node *commit = git_pqueue_pop(&list);
		int flags;

		if (commit == NULL)
			break;

		flags = commit->flags & (PARENT1 | PARENT2 | STALE);
		if (flags == (PARENT1 | PARENT2)) {
			if (!(commit->flags & RESULT)) {
				commit->flags |= RESULT;
				if (git_commit_list_insert(commit, &result) == NULL)
					return -1;
			}
			/* we mark the parents of a merge stale */
			flags |= STALE;
		}

		for (i = 0; i < commit->out_degree; i++) {
			git_commit_list_node *p = commit->parents[i];
			if ((p->flags & flags) == flags)
				continue;
			if (p->generation < minimum_generation)
				continue;

			if ((error = git_commit_list_parse(walk, p)) < 0)
				return error;

			p->flags |= flags;
			if (git_pqueue_insert(&list, p) < 0)
				return -1;
		}


// Source: merge.c
// Lines 380-442
