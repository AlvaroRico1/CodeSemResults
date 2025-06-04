GIT_INLINE(int) tree_iterator_entry_cmp_icase(
	const void *ptr_a, const void *ptr_b)
{
	const tree_iterator_entry *a = (const tree_iterator_entry *)ptr_a;
	const tree_iterator_entry *b = (const tree_iterator_entry *)ptr_b;

	return tree_entry_cmp(a->tree_entry, b->tree_entry, true);
}


// Source: iterator.c
// Lines 482-489
