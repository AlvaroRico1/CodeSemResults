static int handle_rename_delete(struct merge_options *opt,
				struct rename_conflict_info *ci)
{
	const struct rename *ren = ci->ren1;
	const struct diff_filespec *orig = ren->pair->one;
	const struct diff_filespec *dest = ren->pair->two;
	const char *rename_branch = ren->branch;
	const char *delete_branch = (opt->branch1 == ren->branch ?
				     opt->branch2 : opt->branch1);

	if (handle_change_delete(opt,
				 opt->priv->call_depth ? orig->path : dest->path,
				 opt->priv->call_depth ? NULL : orig->path,
				 orig, dest,
				 rename_branch, delete_branch,
				 _("rename"), _("renamed")))
		return -1;

	if (opt->priv->call_depth)
		return remove_file_from_index(opt->repo->index, dest->path);
	else
		return update_stages(opt, dest->path, NULL,
				     rename_branch == opt->branch1 ? dest : NULL,
				     rename_branch == opt->branch1 ? NULL : dest);
}


// Source: merge-recursive.c
// Lines 1544-1568
