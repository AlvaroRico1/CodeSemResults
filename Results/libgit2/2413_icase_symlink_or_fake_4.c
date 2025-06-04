static int symlink_or_fake(git_repository *repo, const char *a, const char *b)
{
	int symlinks;

	cl_git_pass(git_repository__configmap_lookup(&symlinks, repo, GIT_CONFIGMAP_SYMLINKS));

	if (symlinks)
		return p_symlink(a, b);
	else
		return git_futils_fake_symlink(a, b);
}


// Source: icase.c
// Lines 95-105
