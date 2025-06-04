static int remove_redundant(git_revwalk *walk, git_vector *commits, uint32_t minimum_generation)
{
	git_vector work = GIT_VECTOR_INIT;
	unsigned char *redundant;
	unsigned int *filled_index;
	unsigned int i, j;
	int error = 0;

	redundant = git__calloc(commits->length, 1);
	GIT_ERROR_CHECK_ALLOC(redundant);
	filled_index = git__calloc((commits->length - 1), sizeof(unsigned int));
	GIT_ERROR_CHECK_ALLOC(filled_index);

	for (i = 0; i < commits->length; ++i) {
		if ((error = git_commit_list_parse(walk, commits->contents[i])) < 0)
			goto done;
	}

	for (i = 0; i < commits->length; ++i) {
		git_commit_list *common = NULL;
		git_commit_list_node *commit = commits->contents[i];

		if (redundant[i])
			continue;

		git_vector_clear(&work);

		for (j = 0; j < commits->length; j++) {
			if (i == j || redundant[j])
				continue;

			filled_index[work.length] = j;
			if ((error = git_vector_insert(&work, commits->contents[j])) < 0)
				goto done;
		}

		error = paint_down_to_common(&common, walk, commit, &work, minimum_generation);
		if (error < 0)
			goto done;

		if (commit->flags & PARENT2)
			redundant[i] = 1;

		for (j = 0; j < work.length; j++) {
			git_commit_list_node *w = work.contents[j];
			if (w->flags & PARENT1)
				redundant[filled_index[j]] = 1;
		}


// Source: merge.c
// Lines 450-497
