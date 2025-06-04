void test_threads_atomic__load_ptr(void)
{
	int value = 1;
	int *ptr = &value;
	cl_assert_equal_p(git_atomic_load(ptr), &value);
}


// Source: atomic.c
// Lines 114-119
