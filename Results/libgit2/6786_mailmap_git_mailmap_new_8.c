int git_mailmap_new(git_mailmap **out)
{
	int error;
	git_mailmap *mm = git__calloc(1, sizeof(git_mailmap));
	GIT_ERROR_CHECK_ALLOC(mm);

	error = git_vector_init(&mm->entries, 0, mailmap_entry_cmp);
	if (error < 0) {
		git__free(mm);
		return error;
	}
	*out = mm;
	return 0;
}


// Source: mailmap.c
// Lines 151-164
