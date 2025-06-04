int clar_summary_shutdown(struct clar_summary *summary)
{
	struct clar_report *report;
	const char *last_suite = NULL;

	if (clar_summary_testsuites(summary) < 0)
		goto on_error;

	report = _clar.reports;
	while (report != NULL) {
		struct clar_error *error = report->errors;

		if (last_suite == NULL || strcmp(last_suite, report->suite) != 0) {
			if (clar_summary_testsuite(summary, 0, report->suite, "",
			    time(NULL), 0, _clar.tests_ran, _clar.total_errors, 0) < 0)
				goto on_error;
		}

		last_suite = report->suite;

		clar_summary_testcase(summary, report->test, "what", 0);

		while (error != NULL) {
			if (clar_summary_failure(summary, "assert",
			    error->error_msg, error->description) < 0)
				goto on_error;

			error = error->next;
		}

		if (clar_summary_close_tag(summary, "testcase", 2) < 0)
			goto on_error;

		report = report->next;

		if (!report || strcmp(last_suite, report->suite) != 0) {
			if (clar_summary_close_tag(summary, "testsuite", 1) < 0)
				goto on_error;
		}
	}


// Source: summary.h
// Lines 80-119
