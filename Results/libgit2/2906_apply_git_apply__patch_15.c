int git_apply__patch(
	git_str *contents_out,
	char **filename_out,
	unsigned int *mode_out,
	const char *source,
	size_t source_len,
	git_patch *patch,
	const git_apply_options *given_opts)
{
	apply_hunks_ctx ctx = { GIT_APPLY_OPTIONS_INIT };
	char *filename = NULL;
	unsigned int mode = 0;
	int error = 0;

	GIT_ASSERT_ARG(contents_out);
	GIT_ASSERT_ARG(filename_out);
	GIT_ASSERT_ARG(mode_out);
	GIT_ASSERT_ARG(source || !source_len);
	GIT_ASSERT_ARG(patch);

	if (given_opts)
		memcpy(&ctx.opts, given_opts, sizeof(git_apply_options));

	*filename_out = NULL;
	*mode_out = 0;

	if (patch->delta->status != GIT_DELTA_DELETED) {
		const git_diff_file *newfile = &patch->delta->new_file;

		filename = git__strdup(newfile->path);
		mode = newfile->mode ?
			newfile->mode : GIT_FILEMODE_BLOB;
	}

	if (patch->delta->flags & GIT_DIFF_FLAG_BINARY)
		error = apply_binary(contents_out, source, source_len, patch);
	else if (patch->hunks.size)
		error = apply_hunks(contents_out, source, source_len, patch, &ctx);
	else
		error = git_str_put(contents_out, source, source_len);

	if (error)
		goto done;

	if (patch->delta->status == GIT_DELTA_DELETED &&
		git_str_len(contents_out) > 0) {
		error = apply_err("removal patch leaves file contents");
		goto done;
	}

	*filename_out = filename;
	*mode_out = mode;

done:
	if (error < 0)
		git__free(filename);

	return error;
}


// Source: apply.c
// Lines 387-445
