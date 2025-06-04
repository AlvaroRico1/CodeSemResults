void test_refs_iterator__empty(void)
{
	git_reference_iterator *iter;
	git_odb *odb;
	git_reference *ref;
	git_repository *empty;

	cl_git_pass(git_odb_new(&odb));
	cl_git_pass(git_repository_wrap_odb(&empty, odb));

	cl_git_pass(git_reference_iterator_new(&iter, empty));
	cl_assert_equal_i(GIT_ITEROVER, git_reference_next(&ref, iter));

	git_reference_iterator_free(iter);
	git_odb_free(odb);
	git_repository_free(empty);
}


// Source: iterator.c
// Lines 122-138
