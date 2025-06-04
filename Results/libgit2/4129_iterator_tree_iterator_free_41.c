static void tree_iterator_free(git_iterator *i)
{
	tree_iterator *iter = (tree_iterator *)i;

	tree_iterator_clear(iter);

	git_tree_free(iter->root);
	git_str_dispose(&iter->entry_path);
}


// Source: iterator.c
// Lines 922-930
