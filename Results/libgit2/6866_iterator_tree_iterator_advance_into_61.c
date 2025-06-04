static int tree_iterator_advance_into(
	const git_index_entry **out, git_iterator *i)
{
	tree_iterator *iter = (tree_iterator *)i;
	tree_iterator_frame *frame;
	tree_iterator_entry *prev_entry;
	int error;

	if (out)
		*out = NULL;

	if ((frame = tree_iterator_current_frame(iter)) == NULL)
		return GIT_ITEROVER;

	/* get the last seen entry */
	prev_entry = tree_iterator_current_entry(frame);

	/* it's legal to call advance_into when auto-expand is on.  in this case,
	 * we will have pushed a new (empty) frame on to the stack for this
	 * new directory.  since it's empty, its current_entry should be null.
	 */
	GIT_ASSERT(iterator__do_autoexpand(i) ^ (prev_entry != NULL));

	if (prev_entry) {
		if (!git_tree_entry__is_tree(prev_entry->tree_entry))
			return 0;

		if ((error = tree_iterator_frame_push(iter, prev_entry)) < 0)
			return error;
	}

	/* we've advanced into the directory in question, let advance
	 * find the first entry
	 */
	return tree_iterator_advance(out, i);
}


// Source: iterator.c
// Lines 842-877
