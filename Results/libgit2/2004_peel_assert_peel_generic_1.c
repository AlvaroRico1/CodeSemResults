static void assert_peel_generic(
	git_repository *repo,
	const char *ref_name,
	git_object_t requested_type,
	const char* expected_sha,
	git_object_t expected_type)
{
	git_oid expected_oid;
	git_reference *ref;
	git_object *peeled;

	cl_git_pass(git_reference_lookup(&ref, repo, ref_name));

	cl_git_pass(git_reference_peel(&peeled, ref, requested_type));

	cl_git_pass(git_oid_fromstr(&expected_oid, expected_sha));
	cl_assert_equal_oid(&expected_oid, git_object_id(peeled));

	cl_assert_equal_i(expected_type, git_object_type(peeled));

	git_object_free(peeled);
	git_reference_free(ref);
}


// Source: peel.c
// Lines 20-42
