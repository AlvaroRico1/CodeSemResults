const char *git_commit_body(git_commit *commit)
{
	const char *msg, *end;

	GIT_ASSERT_ARG_WITH_RETVAL(commit, NULL);

	if (!commit->body) {
		/* search for end of summary */
		for (msg = git_commit_message(commit); *msg; ++msg)
			if (msg[0] == '\n' && (!msg[1] || msg[1] == '\n'))
				break;

		/* trim leading and trailing whitespace */
		for (; *msg; ++msg)
			if (!git__isspace(*msg))
				break;
		for (end = msg + strlen(msg) - 1; msg <= end; --end)
			if (!git__isspace(*end))
				break;

		if (*msg)
			commit->body = git__strndup(msg, end - msg + 1);
	}

	return commit->body;
}


// Source: commit.c
// Lines 603-628
