clar_test_shutdown(void)
{
	struct clar_explicit *explicit, *explicit_next;
	struct clar_report *report, *report_next;

	clar_print_shutdown(
		_clar.tests_ran,
		(int)_clar_suite_count,
		_clar.total_errors
	);

	clar_unsandbox();

	if (_clar.write_summary && clar_summary_shutdown(_clar.summary) < 0) {
		clar_print_onabort("Failed to write the summary file\n");
		exit(-1);
	}

	for (explicit = _clar.explicit; explicit; explicit = explicit_next) {
		explicit_next = explicit->next;
		free(explicit);
	}

	for (report = _clar.reports; report; report = report_next) {
		report_next = report->next;
		free(report);
	}

	free(_clar.summary_filename);
}


// Source: clar.c
// Lines 551-580
