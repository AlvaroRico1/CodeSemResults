static void verify_patch_id(const char *diff_content, const char *expected_id)
{
	git_oid expected_oid, actual_oid;
	git_diff *diff;

	cl_git_pass(git_oid_fromstr(&expected_oid, expected_id));
	cl_git_pass(git_diff_from_buffer(&diff, diff_content, strlen(diff_content)));
	cl_git_pass(git_diff_patchid(&actual_oid, diff, NULL));

	cl_assert_equal_oid(&expected_oid, &actual_oid);

	git_diff_free(diff);
}


// Source: patchid.c
// Lines 4-16
