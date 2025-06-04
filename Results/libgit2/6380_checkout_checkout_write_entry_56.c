static int checkout_write_entry(
	checkout_data *data,
	checkout_conflictdata *conflict,
	const git_index_entry *side)
{
	const char *hint_path = NULL, *suffix;
	git_str *fullpath;
	struct stat st;
	int error;

	GIT_ASSERT(side == conflict->ours || side == conflict->theirs);

	if (checkout_target_fullpath(&fullpath, data, side->path) < 0)
		return -1;

	if ((conflict->name_collision || conflict->directoryfile) &&
		(data->strategy & GIT_CHECKOUT_USE_OURS) == 0 &&
		(data->strategy & GIT_CHECKOUT_USE_THEIRS) == 0) {

		if (side == conflict->ours)
			suffix = data->opts.our_label ? data->opts.our_label :
				"ours";
		else
			suffix = data->opts.their_label ? data->opts.their_label :
				"theirs";

		if (checkout_path_suffixed(fullpath, suffix) < 0)
			return -1;
	}

	hint_path = side->path;

	if ((data->strategy & GIT_CHECKOUT_UPDATE_ONLY) != 0 &&
		(error = checkout_safe_for_update_only(data, fullpath->ptr, side->mode)) <= 0)
		return error;

	if (!S_ISGITLINK(side->mode))
		return checkout_write_content(data,
					      &side->id, fullpath->ptr, hint_path, side->mode, &st);

	return 0;
}


// Source: checkout.c
// Lines 1973-2014
