static void force_create_file(const char *file)
{
	int error = git_futils_rmdir_r(file, NULL,
		GIT_RMDIR_REMOVE_FILES | GIT_RMDIR_REMOVE_BLOCKERS);
	cl_assert(!error || error == GIT_ENOTFOUND);
	cl_git_pass(git_futils_mkpath2file(file, 0777));
	cl_git_rewritefile(file, "yowza!!");
}


// Source: typechange.c
// Lines 218-225
