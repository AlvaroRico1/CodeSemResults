void test_revwalk_mergebase__no_common_ancestor_returns_ENOTFOUND(void)
{
	git_oid result, one, two;
	size_t ahead, behind;
	int error;

	cl_git_pass(git_oid_fromstr(&one, "763d71aadf09a7951596c9746c024e7eece7c7af"));
	cl_git_pass(git_oid_fromstr(&two, "e90810b8df3e80c413d903f631643c716887138d"));

	error = git_merge_base(&result, _repo, &one, &two);
	cl_git_fail(error);

	cl_assert_equal_i(GIT_ENOTFOUND, error);

	cl_git_pass(git_graph_ahead_behind(&ahead, &behind, _repo, &one, &two));
	cl_assert_equal_sz(4, ahead);
	cl_assert_equal_sz(2, behind);
}


// Source: mergebase.c
// Lines 107-124
