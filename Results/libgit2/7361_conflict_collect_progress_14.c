static void collect_progress(
	const char *path,
	size_t completed_steps,
	size_t total_steps,
	void *payload)
{
	git_vector *paths = payload;

	GIT_UNUSED(completed_steps);
	GIT_UNUSED(total_steps);

	if (path == NULL)
		return;

	git_vector_insert(paths, strdup(path));
}


// Source: conflict.c
// Lines 1084-1099
