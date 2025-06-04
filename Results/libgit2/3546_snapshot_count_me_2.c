static int count_me(const git_config_entry *entry, void *payload)
{
	int *n = (int *) payload;

	GIT_UNUSED(entry);

	(*n)++;

	return 0;
}


// Source: snapshot.c
// Lines 51-60
