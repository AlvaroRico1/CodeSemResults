static int diff_hunks(mmfile_t file_a, mmfile_t file_b, void *cb_data, git_blame_options *options)
{
	xdemitconf_t xecfg = {0};
	xdemitcb_t ecb = {0};
	xpparam_t xpp = {0};

	if (options->flags & GIT_BLAME_IGNORE_WHITESPACE)
		xpp.flags |= XDF_IGNORE_WHITESPACE;

	xecfg.hunk_func = my_emit;
	ecb.priv = cb_data;

	trim_common_tail(&file_a, &file_b, 0);

	if (file_a.size > GIT_XDIFF_MAX_SIZE ||
		file_b.size > GIT_XDIFF_MAX_SIZE) {
		git_error_set(GIT_ERROR_INVALID, "file too large to blame");
		return -1;
	}

	return xdl_diff(&file_a, &file_b, &xpp, &xecfg, &ecb);
}


// Source: blame_git.c
// Lines 368-389
