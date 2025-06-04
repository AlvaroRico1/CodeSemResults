static int hunk_skip_odds_cb(const git_diff_hunk *hunk, void *payload)
{
	int *count = (int *)payload;
	GIT_UNUSED(hunk);

	return ((*count)++ % 2 == 1);
}


// Source: callbacks.c
// Lines 92-98
