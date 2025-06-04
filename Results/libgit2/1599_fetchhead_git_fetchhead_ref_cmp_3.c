int git_fetchhead_ref_cmp(const void *a, const void *b)
{
	const git_fetchhead_ref *one = (const git_fetchhead_ref *)a;
	const git_fetchhead_ref *two = (const git_fetchhead_ref *)b;

	if (one->is_merge && !two->is_merge)
		return -1;
	if (two->is_merge && !one->is_merge)
		return 1;

	if (one->ref_name && two->ref_name)
		return strcmp(one->ref_name, two->ref_name);
	else if (one->ref_name)
		return -1;
	else if (two->ref_name)
		return 1;

	return 0;
}


// Source: fetchhead.c
// Lines 20-38
