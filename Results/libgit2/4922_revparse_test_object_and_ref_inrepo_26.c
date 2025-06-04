static void test_object_and_ref_inrepo(
	const char *spec,
	const char *expected_oid,
	const char *expected_refname,
	git_repository *repo,
	bool assert_reference_retrieval)
{
	char objstr[64] = {0};
	git_object *obj = NULL;
	git_reference *ref = NULL;
	int error;

	error = git_revparse_ext(&obj, &ref, repo, spec);

	if (expected_oid != NULL) {
		cl_git_pass(error);
		git_oid_fmt(objstr, git_object_id(obj));
		cl_assert_equal_s(objstr, expected_oid);
	} else
		cl_git_fail(error);

	if (assert_reference_retrieval) {
		if (expected_refname == NULL)
			cl_assert(NULL == ref);
		else
			cl_assert_equal_s(expected_refname, git_reference_name(ref));
	}

	git_object_free(obj);
	git_reference_free(ref);
}


// Source: revparse.c
// Lines 11-41
