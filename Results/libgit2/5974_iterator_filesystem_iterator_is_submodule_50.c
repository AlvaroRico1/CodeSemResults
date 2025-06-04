static int filesystem_iterator_is_submodule(
	bool *out, filesystem_iterator *iter, const char *path, size_t path_len)
{
	bool is_submodule = false;
	int error;

	*out = false;

	/* first see if this path is a submodule in HEAD */
	if (iter->tree) {
		git_tree_entry *entry;

		error = git_tree_entry_bypath(&entry, iter->tree, path);

		if (error < 0 && error != GIT_ENOTFOUND)
			return error;

		if (!error) {
			is_submodule = (entry->attr == GIT_FILEMODE_COMMIT);
			git_tree_entry_free(entry);
		}
	}

	if (!is_submodule && iter->base.index) {
		size_t pos;

		error = git_index_snapshot_find(&pos,
			&iter->index_snapshot, iter->base.entry_srch, path, path_len, 0);

		if (error < 0 && error != GIT_ENOTFOUND)
			return error;

		if (!error) {
			git_index_entry *e = git_vector_get(&iter->index_snapshot, pos);
			is_submodule = (e->mode == GIT_FILEMODE_COMMIT);
		}
	}


// Source: iterator.c
// Lines 1096-1132
