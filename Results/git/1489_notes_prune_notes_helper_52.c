static int prune_notes_helper(const struct object_id *object_oid,
		const struct object_id *note_oid, char *note_path,
		void *cb_data)
{
	struct note_delete_list **l = (struct note_delete_list **) cb_data;
	struct note_delete_list *n;

	if (has_object_file(object_oid))
		return 0; /* nothing to do for this note */

	/* failed to find object => prune this note */
	n = (struct note_delete_list *) xmalloc(sizeof(*n));
	n->next = *l;
	n->sha1 = object_oid->hash;
	*l = n;
	return 0;
}


// Source: notes.c
// Lines 782-798
