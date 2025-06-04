void prune_notes(struct notes_tree *t, int flags)
{
	struct note_delete_list *l = NULL;

	if (!t)
		t = &default_notes_tree;
	assert(t->initialized);

	for_each_note(t, 0, prune_notes_helper, &l);

	while (l) {
		if (flags & NOTES_PRUNE_VERBOSE)
			printf("%s\n", hash_to_hex(l->sha1));
		if (!(flags & NOTES_PRUNE_DRYRUN))
			remove_note(t, l->sha1);
		l = l->next;
	}
}


// Source: notes.c
// Lines 1200-1217
