static void tree_iterator_set_current(
	tree_iterator *iter,
	tree_iterator_frame *frame,
	tree_iterator_entry *entry)
{
	git_tree_entry *tree_entry = entry->tree_entry;

	frame->current = entry;

	memset(&iter->entry, 0x0, sizeof(git_index_entry));

	iter->entry.mode = tree_entry->attr;
	iter->entry.path = iter->entry_path.ptr;
	git_oid_cpy(&iter->entry.id, tree_entry->oid);
}


// Source: iterator.c
// Lines 730-744
