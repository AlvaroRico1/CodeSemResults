static int tree_iterator_frame_init(
	tree_iterator *iter,
	git_tree *tree,
	tree_iterator_entry *frame_entry)
{
	tree_iterator_frame *new_frame = NULL;
	tree_iterator_entry *new_entry;
	git_tree *dup = NULL;
	git_tree_entry *tree_entry;
	git_vector_cmp cmp;
	size_t i;
	int error = 0;

	new_frame = git_array_alloc(iter->frames);
	GIT_ERROR_CHECK_ALLOC(new_frame);

	if ((error = git_tree_dup(&dup, tree)) < 0)
		goto done;

	memset(new_frame, 0x0, sizeof(tree_iterator_frame));
	new_frame->tree = dup;

	if (frame_entry &&
	    (error = tree_iterator_compute_path(&new_frame->path, frame_entry)) < 0)
		goto done;

	cmp = iterator__ignore_case(&iter->base) ?
		tree_iterator_entry_sort_icase : NULL;

	if ((error = git_vector_init(&new_frame->entries,
				     dup->entries.size, cmp)) < 0)
		goto done;

	git_array_foreach(dup->entries, i, tree_entry) {
		if ((new_entry = git_pool_malloc(&iter->entry_pool, 1)) == NULL) {
			git_error_set_oom();
			error = -1;
			goto done;
		}

		new_entry->tree_entry = tree_entry;
		new_entry->parent_path = new_frame->path.ptr;

		if ((error = git_vector_insert(&new_frame->entries, new_entry)) < 0)
			goto done;
	}

	git_vector_set_sorted(&new_frame->entries,
		!iterator__ignore_case(&iter->base));

done:
	if (error < 0) {
		git_tree_free(dup);
		git_array_pop(iter->frames);
	}

	return error;
}


// Source: iterator.c
// Lines 531-588
