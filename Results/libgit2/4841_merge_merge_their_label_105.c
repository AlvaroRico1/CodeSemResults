static const char *merge_their_label(const char *branchname)
{
	const char *slash;

	if ((slash = strrchr(branchname, '/')) == NULL)
		return branchname;

	if (*(slash+1) == '\0')
		return "theirs";

	return slash+1;
}


// Source: merge.c
// Lines 2890-2901
