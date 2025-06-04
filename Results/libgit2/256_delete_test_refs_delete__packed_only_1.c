void test_refs_delete__packed_only(void)
{
	/* can delete a just packed reference */
	git_reference *ref;
	git_refdb *refdb;
	git_oid id;
	const char *new_ref = "refs/heads/new_ref";

	git_oid_fromstr(&id, current_master_tip);

	/* Create and write the new object id reference */
	cl_git_pass(git_reference_create(&ref, g_repo, new_ref, &id, 0, NULL));
	git_reference_free(ref);

	/* Lookup the reference */
	cl_git_pass(git_reference_lookup(&ref, g_repo, new_ref));

	/* Ensure it's a loose reference */
	cl_assert(reference_is_packed(ref) == 0);

	/* Pack all existing references */
	cl_git_pass(git_repository_refdb(&refdb, g_repo));
	cl_git_pass(git_refdb_compress(refdb));

	/* Reload the reference from disk */
	git_reference_free(ref);
	cl_git_pass(git_reference_lookup(&ref, g_repo, new_ref));

	/* Ensure it's a packed reference */
	cl_assert(reference_is_packed(ref) == 1);

	/* This should pass */
	cl_git_pass(git_reference_delete(ref));
	git_reference_free(ref);
	git_refdb_free(refdb);
}


// Source: delete.c
// Lines 58-93
