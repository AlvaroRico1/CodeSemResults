int git_iterator_current_parent_tree(
	const git_tree **parent_tree, git_iterator *i, size_t depth)
{
	tree_iterator *iter;
	tree_iterator_frame *frame;

	GIT_ASSERT(i->type == GIT_ITERATOR_TREE);

	iter = (tree_iterator *)i;

	GIT_ASSERT(depth < iter->frames.size);
	frame = &iter->frames.ptr[iter->frames.size-depth-1];

	*parent_tree = frame->tree;
	return 0;
}


// Source: iterator.c
// Lines 992-1007
