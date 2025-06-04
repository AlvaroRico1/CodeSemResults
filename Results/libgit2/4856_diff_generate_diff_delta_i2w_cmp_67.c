static int diff_delta_i2w_cmp(const void *a, const void *b)
{
	const git_diff_delta *da = a, *db = b;
	int val = strcmp(diff_delta__i2w_path(da), diff_delta__i2w_path(db));
	return val ? val : ((int)da->status - (int)db->status);
}


// Source: diff_generate.c
// Lines 328-333
