void test_revwalk_mergebase__merged_branch(void)
{
	git_oid result, one, two, expected;
	size_t ahead, behind;

	cl_git_pass(git_oid_fromstr(&one, "a65fedf39aefe402d3bb6e24df4d4f5fe4547750"));
	cl_git_pass(git_oid_fromstr(&two, "9fd738e8f7967c078dceed8190330fc8648ee56a"));
	cl_git_pass(git_oid_fromstr(&expected, "9fd738e8f7967c078dceed8190330fc8648ee56a"));

	cl_git_pass(git_merge_base(&result, _repo, &one, &two));
	cl_assert_equal_oid(&expected, &result);

	cl_git_pass(git_merge_base(&result, _repo, &two, &one));
	cl_assert_equal_oid(&expected, &result);

	cl_git_pass(git_graph_ahead_behind(&ahead, &behind, _repo, &one, &two));
	cl_assert_equal_sz(ahead,  3);
	cl_assert_equal_sz(behind,  0);

	cl_git_pass(git_graph_ahead_behind(&ahead, &behind, _repo, &two, &one));
	cl_assert_equal_sz(ahead,  0);
	cl_assert_equal_sz(behind,  3);
}


// Source: mergebase.c
// Lines 65-87
