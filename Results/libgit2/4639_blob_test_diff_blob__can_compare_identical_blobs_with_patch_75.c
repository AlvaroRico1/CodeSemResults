void test_diff_blob__can_compare_identical_blobs_with_patch(void)
{
	git_patch *p;
	const git_diff_delta *delta;

	cl_git_pass(git_patch_from_blobs(&p, d, NULL, d, NULL, &opts));
	cl_assert(p != NULL);

	delta = git_patch_get_delta(p);
	cl_assert(delta != NULL);
	cl_assert_equal_i(GIT_DELTA_UNMODIFIED, delta->status);
	cl_assert_equal_sz(delta->old_file.size, git_blob_rawsize(d));
	cl_assert_equal_oid(git_blob_id(d), &delta->old_file.id);
	cl_assert_equal_sz(delta->new_file.size, git_blob_rawsize(d));
	cl_assert_equal_oid(git_blob_id(d), &delta->new_file.id);

	cl_assert_equal_i(0, (int)git_patch_num_hunks(p));
	git_patch_free(p);

	cl_git_pass(git_patch_from_blobs(&p, NULL, NULL, NULL, NULL, &opts));
	cl_assert(p != NULL);

	delta = git_patch_get_delta(p);
	cl_assert(delta != NULL);
	cl_assert_equal_i(GIT_DELTA_UNMODIFIED, delta->status);
	cl_assert_equal_sz(0, delta->old_file.size);
	cl_assert(git_oid_is_zero(&delta->old_file.id));
	cl_assert_equal_sz(0, delta->new_file.size);
	cl_assert(git_oid_is_zero(&delta->new_file.id));

	cl_assert_equal_i(0, (int)git_patch_num_hunks(p));
	git_patch_free(p);

	cl_git_pass(git_patch_from_blobs(&p, alien, NULL, alien, NULL, &opts));
	cl_assert(p != NULL);
	cl_assert_equal_i(GIT_DELTA_UNMODIFIED, git_patch_get_delta(p)->status);
	cl_assert_equal_i(0, (int)git_patch_num_hunks(p));
	git_patch_free(p);
}


// Source: blob.c
// Lines 422-460
