void test_threads_atomic__cas_pointer(void)
{
	int *value = NULL;
	int newvalue1 = 1, newvalue2 = 2;

	/* value is updated */
	cl_assert_equal_p(git_atomic_compare_and_swap(&value, NULL, &newvalue1), NULL);
	cl_assert_equal_p(value, &newvalue1);

	/* value is not updated */
	cl_assert_equal_p(git_atomic_compare_and_swap(&value, NULL, &newvalue2), &newvalue1);
	cl_assert_equal_p(value, &newvalue1);
}


// Source: atomic.c
// Lines 69-81
