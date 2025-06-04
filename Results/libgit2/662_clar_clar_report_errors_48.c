clar_report_errors(struct clar_report *report)
{
	struct clar_error *error;
	int i = 1;

	for (error = report->errors; error; error = error->next)
		clar_print_error(i++, _clar.last_report, error);
}


// Source: clar.c
// Lines 223-230
