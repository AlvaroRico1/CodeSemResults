static int rebase_open_merge(git_rebase *rebase)
{
	git_str state_path = GIT_STR_INIT, buf = GIT_STR_INIT, cmt = GIT_STR_INIT;
	git_oid id;
	git_rebase_operation *operation;
	size_t i, msgnum = 0, end;
	int error;

	if ((error = git_str_puts(&state_path, rebase->state_path)) < 0)
		goto done;

	/* Read 'msgnum' if it exists (otherwise, let msgnum = 0) */
	if ((error = rebase_readint(&msgnum, &buf, &state_path, MSGNUM_FILE)) < 0 &&
		error != GIT_ENOTFOUND)
		goto done;

	if (msgnum) {
		rebase->started = 1;
		rebase->current = msgnum - 1;
	}

	/* Read 'end' */
	if ((error = rebase_readint(&end, &buf, &state_path, END_FILE)) < 0)
		goto done;

	/* Read 'current' if it exists */
	if ((error = rebase_readoid(&id, &buf, &state_path, CURRENT_FILE)) < 0 &&
		error != GIT_ENOTFOUND)
		goto done;

	/* Read cmt.* */
	git_array_init_to_size(rebase->operations, end);
	GIT_ERROR_CHECK_ARRAY(rebase->operations);

	for (i = 0; i < end; i++) {
		git_str_clear(&cmt);

		if ((error = git_str_printf(&cmt, "cmt.%" PRIuZ, (i+1))) < 0 ||
			(error = rebase_readoid(&id, &buf, &state_path, cmt.ptr)) < 0)
			goto done;

		operation = rebase_operation_alloc(rebase, GIT_REBASE_OPERATION_PICK, &id, NULL);
		GIT_ERROR_CHECK_ALLOC(operation);
	}

	/* Read 'onto_name' */
	if ((error = rebase_readfile(&buf, &state_path, ONTO_NAME_FILE)) < 0)
		goto done;

	rebase->onto_name = git_str_detach(&buf);

done:
	git_str_dispose(&cmt);
	git_str_dispose(&state_path);
	git_str_dispose(&buf);

	return error;
}


// Source: rebase.c
// Lines 203-260
