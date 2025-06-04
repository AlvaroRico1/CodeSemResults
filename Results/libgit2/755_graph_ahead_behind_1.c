static int ahead_behind(git_commit_list_node *one, git_commit_list_node *two,
	size_t *ahead, size_t *behind)
{
	git_commit_list_node *commit;
	git_pqueue pq;
	int error = 0, i;
	*ahead = 0;
	*behind = 0;

	if (git_pqueue_init(&pq, 0, 2, git_commit_list_time_cmp) < 0)
		return -1;

	if ((error = git_pqueue_insert(&pq, one)) < 0 ||
		(error = git_pqueue_insert(&pq, two)) < 0)
		goto done;

	while ((commit = git_pqueue_pop(&pq)) != NULL) {
		if (commit->flags & RESULT ||
			(commit->flags & (PARENT1 | PARENT2)) == (PARENT1 | PARENT2))
			continue;
		else if (commit->flags & PARENT1)
			(*ahead)++;
		else if (commit->flags & PARENT2)
			(*behind)++;

		for (i = 0; i < commit->out_degree; i++) {
			git_commit_list_node *p = commit->parents[i];
			if ((error = git_pqueue_insert(&pq, p)) < 0)
				goto done;
		}


// Source: graph.c
// Lines 108-137
