static void _load_trace_params(void)
{
	char *sz_level;
	char *sz_method;
	char *sz_tests;

	s_trace_loaded = 1;

	sz_level = cl_getenv("CLAR_TRACE_LEVEL");
	if (!sz_level || !*sz_level) {
		s_trace_level = GIT_TRACE_NONE;
		s_trace_method = NULL;
		return;
	}

	/* TODO Parse sz_level and set s_trace_level. */
	s_trace_level = GIT_TRACE_TRACE;

	sz_method = cl_getenv("CLAR_TRACE_METHOD");
	if (set_method(sz_method) < 0)
		set_method(NULL);

	sz_tests = cl_getenv("CLAR_TRACE_TESTS");
	if (sz_tests != NULL)
		s_trace_tests = 1;
}


// Source: clar_libgit2_trace.c
// Lines 114-139
