static void clar_print_tap_ontest(const char *test_name, int test_number, enum cl_test_status status)
{
	const struct clar_error *error = _clar.last_report->errors;

	(void)test_name;
	(void)test_number;

	switch(status) {
	case CL_TEST_OK:
		printf("ok %d - %s::%s\n", test_number, _clar.active_suite, test_name);
		break;
	case CL_TEST_FAILURE:
		printf("not ok %d - %s::%s\n", test_number, _clar.active_suite, test_name);

		printf("    ---\n");
		printf("    reason: |\n");
		printf("      %s\n", error->error_msg);

		if (error->description)
			printf("      %s\n", error->description);

		printf("    at:\n");
		printf("      file: '"); print_escaped(error->file); printf("'\n");
		printf("      line: %" PRIuZ "\n", error->line_number);
		printf("      function: '%s'\n", error->function);
		printf("    ---\n");

		break;
	case CL_TEST_SKIP:
	case CL_TEST_NOTRUN:
		printf("ok %d - # SKIP %s::%s\n", test_number, _clar.active_suite, test_name);
		break;
	}

	fflush(stdout);
}


// Source: print.h
// Lines 105-140
