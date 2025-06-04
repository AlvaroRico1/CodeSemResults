static unsigned long finish_depth_computation(
	git_pqueue *list,
	git_revwalk *walk,
	struct possible_tag *best)
{
	unsigned long seen_commits = 0;
	int error, i;

	while (git_pqueue_size(list) > 0) {
		git_commit_list_node *c = git_pqueue_pop(list);
		seen_commits++;
		if (c->flags & best->flag_within) {
			size_t index = 0;
			while (git_pqueue_size(list) > index) {
				git_commit_list_node *i = git_pqueue_get(list, index);
				if (!(i->flags & best->flag_within))
					break;
				index++;
			}
			if (index > git_pqueue_size(list))
				break;
		} else
			best->depth++;
		for (i = 0; i < c->out_degree; i++) {
			git_commit_list_node *p = c->parents[i];
			if ((error = git_commit_list_parse(walk, p)) < 0)
				return error;
			if (!(p->flags & SEEN))
				if ((error = git_pqueue_insert(list, p)) < 0)
					return error;
			p->flags |= c->flags;
		}


// Source: describe.c
// Lines 290-321
