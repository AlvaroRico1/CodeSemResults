GIT_INLINE(unsigned int) dotgit_flags(
	git_repository *repo,
	unsigned int flags)
{
	int protectHFS = 0, protectNTFS = 1;
	int error = 0;

	flags |= GIT_PATH_REJECT_DOT_GIT_LITERAL;

#ifdef __APPLE__
	protectHFS = 1;
#endif

	if (repo && !protectHFS)
		error = git_repository__configmap_lookup(&protectHFS, repo, GIT_CONFIGMAP_PROTECTHFS);
	if (!error && protectHFS)
		flags |= GIT_PATH_REJECT_DOT_GIT_HFS;

	if (repo)
		error = git_repository__configmap_lookup(&protectNTFS, repo, GIT_CONFIGMAP_PROTECTNTFS);
	if (!error && protectNTFS)
		flags |= GIT_PATH_REJECT_DOT_GIT_NTFS;

	return flags;
}


// Source: path.c
// Lines 263-287
