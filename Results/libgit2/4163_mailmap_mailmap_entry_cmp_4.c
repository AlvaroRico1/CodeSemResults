static int mailmap_entry_cmp(const void *a_raw, const void *b_raw)
{
	const git_mailmap_entry *a = (const git_mailmap_entry *)a_raw;
	const git_mailmap_entry *b = (const git_mailmap_entry *)b_raw;
	int cmp;

	GIT_ASSERT_ARG(a && a->replace_email);
	GIT_ASSERT_ARG(b && b->replace_email);

	cmp = git__strcmp(a->replace_email, b->replace_email);
	if (cmp)
		return cmp;

	/* NULL replace_names are less than not-NULL ones */
	if (a->replace_name == NULL || b->replace_name == NULL)
		return (int)(a->replace_name != NULL) - (int)(b->replace_name != NULL);

	return git__strcmp(a->replace_name, b->replace_name);
}


// Source: mailmap.c
// Lines 42-60
