static int push_one_ignore(void *payload, const char *path)
{
	git_ignores *ign = payload;
	ign->depth++;
	return push_ignore_file(ign, &ign->ign_path, path, GIT_IGNORE_FILE);
}


// Source: ignore.c
// Lines 268-273
