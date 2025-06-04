static int read_tree_cb(
	const char *root, const git_tree_entry *tentry, void *payload)
{
	read_tree_data *data = payload;
	git_index_entry *entry = NULL, *old_entry;
	git_str path = GIT_STR_INIT;
	size_t pos;

	if (git_tree_entry__is_tree(tentry))
		return 0;

	if (git_str_joinpath(&path, root, tentry->filename) < 0)
		return -1;

	if (index_entry_create(&entry, INDEX_OWNER(data->index), path.ptr, NULL, false) < 0)
		return -1;

	entry->mode = tentry->attr;
	git_oid_cpy(&entry->id, git_tree_entry_id(tentry));

	/* look for corresponding old entry and copy data to new entry */
	if (data->old_entries != NULL &&
		!index_find_in_entries(
			&pos, data->old_entries, data->entry_cmp, path.ptr, 0, 0) &&
		(old_entry = git_vector_get(data->old_entries, pos)) != NULL &&
		entry->mode == old_entry->mode &&
		git_oid_equal(&entry->id, &old_entry->id))
	{
		index_entry_cpy(entry, old_entry);
		entry->flags_extended = 0;
	}

	index_entry_adjust_namemask(entry, path.size);
	git_str_dispose(&path);

	if (git_vector_insert(data->new_entries, entry) < 0) {
		index_entry_free(entry);
		return -1;
	}

	return 0;
}


// Source: index.c
// Lines 3111-3152
