static int write_each_note(const struct object_id *object_oid,
		const struct object_id *note_oid, char *note_path,
		void *cb_data)
{
	struct write_each_note_data *d =
		(struct write_each_note_data *) cb_data;
	size_t note_path_len = strlen(note_path);
	unsigned int mode = 0100644;

	if (note_path[note_path_len - 1] == '/') {
		/* subtree entry */
		note_path_len--;
		note_path[note_path_len] = '\0';
		mode = 040000;
	}
	assert(note_path_len <= GIT_MAX_HEXSZ + FANOUT_PATH_SEPARATORS);

	/* Weave non-note entries into note entries */
	return  write_each_non_note_until(note_path, d) ||
		write_each_note_helper(d->root, note_path, mode, note_oid);
}


// Source: notes.c
// Lines 755-775
