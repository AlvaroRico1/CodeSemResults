static void record_entry_for_tree(struct directory_versions *dir_metadata,
				  const char *path,
				  struct merged_info *mi)
{
	const char *basename;

	if (mi->is_null)
		/* nothing to record */
		return;

	basename = path + mi->basename_offset;
	assert(strchr(basename, '/') == NULL);
	string_list_append(&dir_metadata->versions,
			   basename)->util = &mi->result;
}


// Source: merge-ort.c
// Lines 3360-3374
