clar_report_all(void)
{
	struct clar_report *report;
	struct clar_error *error;
	int i = 1;

	for (report = _clar.reports; report; report = report->next) {
		if (report->status != CL_TEST_FAILURE)
			continue;

		for (error = report->errors; error; error = error->next)
			clar_print_error(i++, report, error);
	}
}


// Source: clar.c
// Lines 233-246
