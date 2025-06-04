static int tree_iterator_reset(git_iterator *i)
{
	tree_iterator *iter = (tree_iterator *)i;

	tree_iterator_clear(iter);
	return tree_iterator_init(iter);
}


// Source: iterator.c
// Lines 914-920
