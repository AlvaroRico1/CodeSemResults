const char *git_commit_summary(git_commit *commit)
{
	git_str summary = GIT_STR_INIT;
	const char *msg, *space, *next;
	bool space_contains_newline = false;

	GIT_ASSERT_ARG_WITH_RETVAL(commit, NULL);

	if (!commit->summary) {
		for (msg = git_commit_message(commit), space = NULL; *msg; ++msg) {
			char next_character = msg[0];
			/* stop processing at the end of the first paragraph */
			if (next_character == '\n') {
				if (!msg[1])
					break;
				if (msg[1] == '\n')
					break;
				/* stop processing if next line contains only whitespace */
				next = msg + 1;
				while (*next && git__isspace_nonlf(*next)) {
					++next;
				}
				if (!*next || *next == '\n') 
					break;
			}
			/* record the beginning of contiguous whitespace runs */
			if (git__isspace(next_character)) {
				if(space == NULL) {
					space = msg;
					space_contains_newline = false;
				}
				space_contains_newline |= next_character == '\n';
			}
			/* the next character is non-space */
			else {
				/* process any recorded whitespace */
				if (space) {
					if(space_contains_newline)
						git_str_putc(&summary, ' '); /* if the space contains a newline, collapse to ' ' */
					else
						git_str_put(&summary, space, (msg - space)); /* otherwise copy it */
					space = NULL;
				}
				/* copy the next character */
				git_str_putc(&summary, next_character);
			}
		}

		commit->summary = git_str_detach(&summary);
		if (!commit->summary)
			commit->summary = git__strdup("");
	}

	return commit->summary;
}


// Source: commit.c
// Lines 547-601
