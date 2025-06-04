void clar__fail(
	const char *file,
	const char *function,
	size_t line,
	const char *error_msg,
	const char *description,
	int should_abort)
{
	struct clar_error *error = calloc(1, sizeof(struct clar_error));

	if (_clar.last_report->errors == NULL)
		_clar.last_report->errors = error;

	if (_clar.last_report->last_error != NULL)
		_clar.last_report->last_error->next = error;

	_clar.last_report->last_error = error;

	error->file = file;
	error->function = function;
	error->line_number = line;
	error->error_msg = error_msg;

	if (description != NULL)
		error->description = strdup(description);

	_clar.total_errors++;
	_clar.last_report->status = CL_TEST_FAILURE;

	if (should_abort)
		abort_test();
}


// Source: clar.c
// Lines 614-645
