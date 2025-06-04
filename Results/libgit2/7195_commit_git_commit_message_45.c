const char *git_commit_message(const git_commit *commit)
{
	const char *message;

	GIT_ASSERT_ARG_WITH_RETVAL(commit, NULL);

	message = commit->raw_message;

	/* trim leading newlines from raw message */
	while (*message && *message == '\n')
		++message;

	return message;
}


// Source: commit.c
// Lines 532-545
