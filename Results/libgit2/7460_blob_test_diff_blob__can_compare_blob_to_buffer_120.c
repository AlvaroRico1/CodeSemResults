void test_diff_blob__can_compare_blob_to_buffer(void)
{
	git_blob *a;
	git_oid a_oid;
	const char *a_content = "Hello from the root\n";
	const char *b_content = "Hello from the root\n\nSome additional lines\n\nDown here below\n\n";

	/* tests/resources/attr/root_test1 */
	cl_git_pass(git_oid_fromstrn(&a_oid, "45141a79", 8));
	cl_git_pass(git_blob_lookup_prefix(&a, g_repo, &a_oid, 8));

	/* diff from blob a to content of b */
	quick_diff_blob_to_str(a, NULL, b_content, 0, NULL);
	assert_one_modified(1, 6, 1, 5, 0, &expected);

	/* diff from blob a to content of a */
	opts.flags |= GIT_DIFF_INCLUDE_UNMODIFIED;
	quick_diff_blob_to_str(a, NULL, a_content, 0, NULL);
	assert_identical_blobs_comparison(&expected);

	/* diff from NULL blob to content of a */
	memset(&expected, 0, sizeof(expected));
	quick_diff_blob_to_str(NULL, NULL, a_content, 0, NULL);
	assert_changed_single_one_line_file(&expected, GIT_DELTA_ADDED);

	/* diff from blob a to NULL buffer */
	memset(&expected, 0, sizeof(expected));
	quick_diff_blob_to_str(a, NULL, NULL, 0, NULL);
	assert_changed_single_one_line_file(&expected, GIT_DELTA_DELETED);

	/* diff with reverse */
	opts.flags ^= GIT_DIFF_REVERSE;

	memset(&expected, 0, sizeof(expected));
	quick_diff_blob_to_str(a, NULL, NULL, 0, NULL);
	assert_changed_single_one_line_file(&expected, GIT_DELTA_ADDED);

	git_blob_free(a);
}


// Source: blob.c
// Lines 647-685
