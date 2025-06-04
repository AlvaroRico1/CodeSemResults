void test_threads_tlsdata__threads(void)
{
#ifdef GIT_THREADS
	git_thread thread[THREAD_COUNT];
	git_tlsdata_key tlsdata;
	int i;

	cl_git_pass(git_tlsdata_init(&tlsdata, NULL));

	for (i = 0; i < THREAD_COUNT; i++)
		cl_git_pass(git_thread_create(&thread[i], set_and_get, &tlsdata));

	for (i = 0; i < THREAD_COUNT; i++) {
		void *result;

		cl_git_pass(git_thread_join(&thread[i], &result));
		cl_assert_equal_sz(1, (size_t)result);
	}


// Source: tlsdata.c
// Lines 44-61
