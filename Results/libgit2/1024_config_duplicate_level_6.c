static int duplicate_level(void **old_raw, void *new_raw)
{
	backend_internal **old = (backend_internal **)old_raw;

	GIT_UNUSED(new_raw);

	git_error_set(GIT_ERROR_CONFIG, "there already exists a configuration for the given level (%i)", (int)(*old)->level);
	return GIT_EEXISTS;
}


// Source: config.c
// Lines 219-227
