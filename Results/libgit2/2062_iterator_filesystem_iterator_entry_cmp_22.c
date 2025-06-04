static int filesystem_iterator_entry_cmp(const void *_a, const void *_b)
{
	const filesystem_iterator_entry *a = (const filesystem_iterator_entry *)_a;
	const filesystem_iterator_entry *b = (const filesystem_iterator_entry *)_b;

	return git__strcmp(a->path, b->path);
}


// Source: iterator.c
// Lines 1072-1078
