static int diff_stop_after_2_files(
	const git_diff_delta *delta,
	float progress,
	void *payload)
{
	diff_expects *e = payload;

	GIT_UNUSED(progress);
	GIT_UNUSED(delta);

	e->files++;

	return (e->files == 2);
}


// Source: index.c
// Lines 94-107
