static int object_entry__cb(const git_oid *oid, off64_t offset, void *data)
{
	struct object_entry_cb_state *state = (struct object_entry_cb_state *)data;

	git_midx_entry *entry = git_array_alloc(*state->object_entries_array);
	GIT_ERROR_CHECK_ALLOC(entry);

	git_oid_cpy(&entry->sha1, oid);
	entry->offset = offset;
	entry->pack_index = state->pack_index;

	return 0;
}


// Source: midx.c
// Lines 571-583
