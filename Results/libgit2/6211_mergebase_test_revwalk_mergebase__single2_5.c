void test_revwalk_mergebase__single2(void)
{
	git_oid result, one, two, expected;
	size_t ahead, behind;

	cl_git_pass(git_oid_fromstr(&one, "763d71aadf09a7951596c9746c024e7eece7c7af"));
	cl_git_pass(git_oid_fromstr(&two, "a65fedf39aefe402d3bb6e24df4d4f5fe4547750"));
	cl_git_pass(git_oid_fromstr(&expected, "c47800c7266a2be04c571c04d5a6614691ea99bd"));

	cl_git_pass(git_merge_base(&result, _repo, &one, &two));
	cl_assert_equal_oid(&expected, &result);

	cl_git_pass(git_graph_ahead_behind( &ahead, &behind, _repo, &one, &two));
	cl_assert_equal_sz(ahead,  1);
	cl_assert_equal_sz(behind,  4);

	cl_git_pass(git_graph_ahead_behind( &ahead, &behind, _repo, &two, &one));
	cl_assert_equal_sz(ahead,  4);
	cl_assert_equal_sz(behind,  1);
}


// Source: mergebase.c
// Lines 44-63
