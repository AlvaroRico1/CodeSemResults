static int retrieve_previously_checked_out_branch_or_revision(git_object **out, git_reference **base_ref, git_repository *repo, const char *identifier, size_t position)
{
	git_reference *ref = NULL;
	git_reflog *reflog = NULL;
	git_regexp preg;
	int error = -1;
	size_t i, numentries, cur;
	const git_reflog_entry *entry;
	const char *msg;
	git_str buf = GIT_STR_INIT;

	cur = position;

	if (*identifier != '\0' || *base_ref != NULL)
		return GIT_EINVALIDSPEC;

	if (build_regex(&preg, "checkout: moving from (.*) to .*") < 0)
		return -1;

	if (git_reference_lookup(&ref, repo, GIT_HEAD_FILE) < 0)
		goto cleanup;

	if (git_reflog_read(&reflog, repo, GIT_HEAD_FILE) < 0)
		goto cleanup;

	numentries  = git_reflog_entrycount(reflog);

	for (i = 0; i < numentries; i++) {
		git_regmatch regexmatches[2];

		entry = git_reflog_entry_byindex(reflog, i);
		msg = git_reflog_entry_message(entry);
		if (!msg)
			continue;

		if (git_regexp_search(&preg, msg, 2, regexmatches) < 0)
			continue;

		cur--;

		if (cur > 0)
			continue;

		if ((git_str_put(&buf, msg+regexmatches[1].start, regexmatches[1].end - regexmatches[1].start)) < 0)
			goto cleanup;

		if ((error = git_reference_dwim(base_ref, repo, git_str_cstr(&buf))) == 0)
			goto cleanup;

		if (error < 0 && error != GIT_ENOTFOUND)
			goto cleanup;

		error = maybe_abbrev(out, repo, git_str_cstr(&buf));

		goto cleanup;
	}

	error = GIT_ENOTFOUND;

cleanup:
	git_reference_free(ref);
	git_str_dispose(&buf);
	git_regexp_dispose(&preg);
	git_reflog_free(reflog);
	return error;
}


// Source: revparse.c
// Lines 140-205
