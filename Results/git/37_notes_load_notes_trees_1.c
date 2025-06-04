struct notes_tree **load_notes_trees(struct string_list *refs, int flags)
{
	struct string_list_item *item;
	int counter = 0;
	struct notes_tree **trees;
	ALLOC_ARRAY(trees, refs->nr + 1);
	for_each_string_list_item(item, refs) {
		struct notes_tree *t = xcalloc(1, sizeof(struct notes_tree));
		init_notes(t, item->string, combine_notes_ignore, flags);
		trees[counter++] = t;
	}


// Source: notes.c
// Lines 1035-1045
