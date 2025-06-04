void test_revwalk_mergebase__single1(void)
{
	git_oid result, one, two, expected;
	size_t ahead, behind;

	cl_git_pass(git_oid_fromstr(&one, "c47800c7266a2be04c571c04d5a6614691ea99bd "));
	cl_git_pass(git_oid_fromstr(&two, "9fd738e8f7967c078dceed8190330fc8648ee56a"));
	cl_git_pass(git_oid_fromstr(&expected, "5b5b025afb0b4c913b4c338a42934a3863bf3644"));

	cl_git_pass(git_merge_base(&result, _repo, &one, &two));
	cl_assert_equal_oid(&expected, &result);

	cl_git_pass(git_graph_ahead_behind(&ahead, &behind, _repo, &one, &two));
	cl_assert_equal_sz(ahead, 1);
	cl_assert_equal_sz(behind, 2);

	cl_git_pass(git_graph_ahead_behind(&ahead, &behind, _repo, &two, &one));
	cl_assert_equal_sz(ahead,  2);
	cl_assert_equal_sz(behind,  1);
}


// Source: mergebase.c
// Lines 23-42
