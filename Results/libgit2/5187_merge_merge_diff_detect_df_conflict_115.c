GIT_INLINE(int) merge_diff_detect_df_conflict(
	struct merge_diff_df_data *df_data,
	git_merge_diff *conflict)
{
	const char *cur_path = merge_diff_path(conflict);

	/* Determine if this is a D/F conflict or the child of one */
	if (df_data->df_path &&
		path_is_prefixed(df_data->df_path, cur_path))
		conflict->type = GIT_MERGE_DIFF_DF_CHILD;
	else if(df_data->df_path)
		df_data->df_path = NULL;
	else if (df_data->prev_path &&
		merge_diff_any_side_added_or_modified(df_data->prev_conflict) &&
		merge_diff_any_side_added_or_modified(conflict) &&
		path_is_prefixed(df_data->prev_path, cur_path)) {
		conflict->type = GIT_MERGE_DIFF_DF_CHILD;

		df_data->prev_conflict->type = GIT_MERGE_DIFF_DIRECTORY_FILE;
		df_data->df_path = df_data->prev_path;
	}

	df_data->prev_path = cur_path;
	df_data->prev_conflict = conflict;

	return 0;
}


// Source: merge.c
// Lines 1648-1674
