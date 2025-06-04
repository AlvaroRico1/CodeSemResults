static int handle_rename_rename_1to2(struct merge_options *opt,
				     struct rename_conflict_info *ci)
{
	/* One file was renamed in both branches, but to different names. */
	struct merge_file_info mfi;
	struct diff_filespec *add;
	struct diff_filespec *o = ci->ren1->pair->one;
	struct diff_filespec *a = ci->ren1->pair->two;
	struct diff_filespec *b = ci->ren2->pair->two;
	char *path_desc;

	output(opt, 1, _("CONFLICT (rename/rename): "
	       "Rename \"%s\"->\"%s\" in branch \"%s\" "
	       "rename \"%s\"->\"%s\" in \"%s\"%s"),
	       o->path, a->path, ci->ren1->branch,
	       o->path, b->path, ci->ren2->branch,
	       opt->priv->call_depth ? _(" (left unresolved)") : "");

	path_desc = xstrfmt("%s and %s, both renamed from %s",
			    a->path, b->path, o->path);
	if (merge_mode_and_contents(opt, o, a, b, path_desc,
				    ci->ren1->branch, ci->ren2->branch,
				    opt->priv->call_depth * 2, &mfi))
		return -1;
	free(path_desc);

	if (opt->priv->call_depth)
		remove_file_from_index(opt->repo->index, o->path);

	/*
	 * For each destination path, we need to see if there is a
	 * rename/add collision.  If not, we can write the file out
	 * to the specified location.
	 */
	add = &ci->ren1->dst_entry->stages[flip_stage(2)];
	if (is_valid(add)) {
		add->path = mfi.blob.path = a->path;
		if (handle_file_collision(opt, a->path,
					  NULL, NULL,
					  ci->ren1->branch,
					  ci->ren2->branch,
					  &mfi.blob, add) < 0)
			return -1;
	} else {
		char *new_path = find_path_for_conflict(opt, a->path,
							ci->ren1->branch,
							ci->ren2->branch);
		if (update_file(opt, 0, &mfi.blob,
				new_path ? new_path : a->path))
			return -1;
		free(new_path);
		if (!opt->priv->call_depth &&
		    update_stages(opt, a->path, NULL, a, NULL))
			return -1;
	}

	if (!mfi.clean && mfi.blob.mode == a->mode &&
	    oideq(&mfi.blob.oid, &a->oid)) {
		/*
		 * Getting here means we were attempting to merge a binary
		 * blob.  Since we can't merge binaries, the merge algorithm
		 * just takes one side.  But we don't want to copy the
		 * contents of one side to both paths; we'd rather use the
		 * original content at the given path for each path.
		 */
		oidcpy(&mfi.blob.oid, &b->oid);
		mfi.blob.mode = b->mode;
	}
	add = &ci->ren2->dst_entry->stages[flip_stage(3)];
	if (is_valid(add)) {
		add->path = mfi.blob.path = b->path;
		if (handle_file_collision(opt, b->path,
					  NULL, NULL,
					  ci->ren1->branch,
					  ci->ren2->branch,
					  add, &mfi.blob) < 0)
			return -1;
	} else {
		char *new_path = find_path_for_conflict(opt, b->path,
							ci->ren2->branch,
							ci->ren1->branch);
		if (update_file(opt, 0, &mfi.blob,
				new_path ? new_path : b->path))
			return -1;
		free(new_path);
		if (!opt->priv->call_depth &&
		    update_stages(opt, b->path, NULL, NULL, b))
			return -1;
	}

	return 0;
}


// Source: merge-recursive.c
// Lines 1730-1821
