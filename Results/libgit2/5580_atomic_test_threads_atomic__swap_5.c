void test_threads_atomic__swap(void)
{
	int *value = NULL;
	int newvalue = 1;

	cl_assert_equal_p(git_atomic_swap(value, &newvalue), NULL);
	cl_assert_equal_p(value, &newvalue);

	cl_assert_equal_p(git_atomic_swap(value, NULL), &newvalue);
	cl_assert_equal_p(value, NULL);
}


// Source: atomic.c
// Lines 102-112
