static int describe(
	struct get_name_data *data,
	git_commit *commit)
{
	struct commit_name *n;
	struct possible_tag *best;
	bool all, tags;
	git_revwalk *walk = NULL;
	git_pqueue list;
	git_commit_list_node *cmit, *gave_up_on = NULL;
	git_vector all_matches = GIT_VECTOR_INIT;
	unsigned int match_cnt = 0, annotated_cnt = 0, cur_match;
	unsigned long seen_commits = 0;	/* TODO: Check long */
	unsigned int unannotated_cnt = 0;
	int error;

	if (git_vector_init(&all_matches, MAX_CANDIDATES_TAGS, compare_pt) < 0)
		return -1;

	if ((error = git_pqueue_init(&list, 0, 2, git_commit_list_time_cmp)) < 0)
		goto cleanup;

	all = data->opts->describe_strategy == GIT_DESCRIBE_ALL;
	tags = data->opts->describe_strategy == GIT_DESCRIBE_TAGS;

	git_oid_cpy(&data->result->commit_id, git_commit_id(commit));

	n = find_commit_name(data->names, git_commit_id(commit));
	if (n && (tags || all || n->prio == 2)) {
		/*
		 * Exact match to an existing ref.
		 */
		data->result->exact_match = 1;
		if ((error = commit_name_dup(&data->result->name, n)) < 0)
			goto cleanup;

		goto cleanup;
	}

	if (!data->opts->max_candidates_tags) {
		error = describe_not_found(
			git_commit_id(commit),
			"cannot describe - no tag exactly matches '%s'");

		goto cleanup;
	}

	if ((error = git_revwalk_new(&walk, git_commit_owner(commit))) < 0)
		goto cleanup;

	if ((cmit = git_revwalk__commit_lookup(walk, git_commit_id(commit))) == NULL)
		goto cleanup;

	if ((error = git_commit_list_parse(walk, cmit)) < 0)
		goto cleanup;

	cmit->flags = SEEN;

	if ((error = git_pqueue_insert(&list, cmit)) < 0)
		goto cleanup;

	while (git_pqueue_size(&list) > 0)
	{
		int i;

		git_commit_list_node *c = (git_commit_list_node *)git_pqueue_pop(&list);
		seen_commits++;

		n = find_commit_name(data->names, &c->oid);

		if (n) {
			if (!tags && !all && n->prio < 2) {
				unannotated_cnt++;
			} else if (match_cnt < data->opts->max_candidates_tags) {
				struct possible_tag *t = git__malloc(sizeof(struct commit_name));
				GIT_ERROR_CHECK_ALLOC(t);
				if ((error = git_vector_insert(&all_matches, t)) < 0)
					goto cleanup;

				match_cnt++;

				t->name = n;
				t->depth = seen_commits - 1;
				t->flag_within = 1u << match_cnt;
				t->found_order = match_cnt;
				c->flags |= t->flag_within;
				if (n->prio == 2)
					annotated_cnt++;
			}


// Source: describe.c
// Lines 424-512
