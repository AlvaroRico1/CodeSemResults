void test_refs_read__packed(void)
{
	/* lookup a packed reference */
	git_reference *reference;
	git_object *object;

	cl_git_pass(git_reference_lookup(&reference, g_repo, packed_head_name));
	cl_assert(git_reference_type(reference) & GIT_REFERENCE_DIRECT);
	cl_assert(reference_is_packed(reference));
	cl_assert_equal_s(reference->name, packed_head_name);

	cl_git_pass(git_object_lookup(&object, g_repo, git_reference_target(reference), GIT_OBJECT_ANY));
	cl_assert(object != NULL);
	cl_assert(git_object_type(object) == GIT_OBJECT_COMMIT);

	git_object_free(object);

	git_reference_free(reference);
}


// Source: read.c
// Lines 163-181
