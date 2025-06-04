static void diff_stats_from_commit_oid(
	git_diff_stats **stats, const char *oidstr, bool rename)
{
	git_oid oid;
	git_commit *commit;
	git_diff *diff;

	git_oid_fromstr(&oid, oidstr);
	cl_git_pass(git_commit_lookup(&commit, _repo, &oid));
	cl_git_pass(git_diff__commit(&diff, _repo, commit, NULL));
	if (rename)
		cl_git_pass(git_diff_find_similar(diff, NULL));
	cl_git_pass(git_diff_get_stats(stats, diff));

	git_diff_free(diff);
	git_commit_free(commit);
}


// Source: stats.c
// Lines 22-38
