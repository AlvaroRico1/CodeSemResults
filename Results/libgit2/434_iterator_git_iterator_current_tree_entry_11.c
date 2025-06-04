int git_iterator_current_tree_entry(
	const git_tree_entry **tree_entry, git_iterator *i)
{
	tree_iterator *iter;
	tree_iterator_frame *frame;
	tree_iterator_entry *entry;

	GIT_ASSERT(i->type == GIT_ITERATOR_TREE);

	iter = (tree_iterator *)i;

	frame = tree_iterator_current_frame(iter);
	entry = tree_iterator_current_entry(frame);

	*tree_entry = entry->tree_entry;
	return 0;
}


// Source: iterator.c
// Lines 974-990
