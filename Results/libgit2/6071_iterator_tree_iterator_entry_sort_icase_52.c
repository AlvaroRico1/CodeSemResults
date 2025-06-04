static int tree_iterator_entry_sort_icase(const void *ptr_a, const void *ptr_b)
{
	const tree_iterator_entry *a = (const tree_iterator_entry *)ptr_a;
	const tree_iterator_entry *b = (const tree_iterator_entry *)ptr_b;

	int c = tree_entry_cmp(a->tree_entry, b->tree_entry, true);

	/* stabilize the sort order for filenames that are (case insensitively)
	 * the same by examining the parent path (case sensitively) before
	 * falling back to a case sensitive sort of the filename.
	 */
	if (!c && a->parent_path != b->parent_path)
		c = git__strcmp(a->parent_path, b->parent_path);

	if (!c)
		c = tree_entry_cmp(a->tree_entry, b->tree_entry, false);

	return c;
}


// Source: iterator.c
// Lines 491-509
