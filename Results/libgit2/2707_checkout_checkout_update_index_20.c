static int checkout_update_index(
	checkout_data *data,
	const git_diff_file *file,
	struct stat *st)
{
	git_index_entry entry;

	if (!data->index)
		return 0;

	memset(&entry, 0, sizeof(entry));
	entry.path = (char *)file->path; /* cast to prevent warning */
	git_index_entry__init_from_stat(&entry, st, true);
	git_oid_cpy(&entry.id, &file->id);

	return git_index_add(data->index, &entry);
}


// Source: checkout.c
// Lines 1618-1634
