static void assert_name_is(const char *expected)
{
	char *actual;
	size_t actual_len, expected_len, start;

	cl_assert(actual = get_filename(expected));

	expected_len = strlen(expected);
	actual_len = strlen(actual);
	cl_assert(actual_len >= expected_len);

	start = actual_len - expected_len;
	cl_assert_equal_s(expected, actual + start);

	if (start)
		cl_assert_equal_strn("/", actual + (start - 1), 1);

	free(actual);
}


// Source: icase.c
// Lines 75-93
