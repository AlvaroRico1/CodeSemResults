static struct strmap_entry *check_dir_renamed(const char *path,
					      struct strmap *dir_renames)
{
	char *temp = xstrdup(path);
	char *end;
	struct strmap_entry *e = NULL;

	while ((end = strrchr(temp, '/'))) {
		*end = '\0';
		e = strmap_get_entry(dir_renames, temp);
		if (e)
			break;
	}
	free(temp);
	return e;
}

static void compute_collisions(struct strmap *collisions,
			       struct strmap *dir_renames,
			       struct diff_queue_struct *pairs)
{
	int i;

	strmap_init_with_options(collisions, NULL, 0);
	if (strmap_empty(dir_renames))
		return;

	/*
	 * Multiple files can be mapped to the same path due to directory
	 * renames done by the other side of history.  Since that other
	 * side of history could have merged multiple directories into one,
	 * if our side of history added the same file basename to each of
	 * those directories, then all N of them would get implicitly
	 * renamed by the directory rename detection into the same path,
	 * and we'd get an add/add/.../add conflict, and all those adds
	 * from *this* side of history.  This is not representable in the
	 * index, and users aren't going to easily be able to make sense of
	 * it.  So we need to provide a good warning about what's
	 * happening, and fall back to no-directory-rename detection
	 * behavior for those paths.
	 *
	 * See testcases 9e and all of section 5 from t6043 for examples.
	 */
	for (i = 0; i < pairs->nr; ++i) {
		struct strmap_entry *rename_info;
		struct collision_info *collision_info;
		char *new_path;
		struct diff_filepair *pair = pairs->queue[i];

		if (pair->status != 'A' && pair->status != 'R')
			continue;
		rename_info = check_dir_renamed(pair->two->path, dir_renames);
		if (!rename_info)
			continue;

		new_path = apply_dir_rename(rename_info, pair->two->path);
		assert(new_path);
		collision_info = strmap_get(collisions, new_path);
		if (collision_info) {
			free(new_path);
		} else {
			CALLOC_ARRAY(collision_info, 1);
			string_list_init_nodup(&collision_info->source_files);
			strmap_put(collisions, new_path, collision_info);
		}
		string_list_insert(&collision_info->source_files,
				   pair->two->path);
	}
}


// Source: merge-ort.c
// Lines 2152-2220
