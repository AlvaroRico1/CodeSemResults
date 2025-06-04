static void write_note_to_worktree(const struct object_id *obj,
				   const struct object_id *note)
{
	enum object_type type;
	unsigned long size;
	void *buf = read_object_file(note, &type, &size);

	if (!buf)
		die("cannot read note %s for object %s",
		    oid_to_hex(note), oid_to_hex(obj));
	if (type != OBJ_BLOB)
		die("blob expected in note %s for object %s",
		    oid_to_hex(note), oid_to_hex(obj));
	write_buf_to_worktree(obj, buf, size);
	free(buf);
}


// Source: notes-merge.c
// Lines 325-340
