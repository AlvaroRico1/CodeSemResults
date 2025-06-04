clar_run_suite(const struct clar_suite *suite, const char *filter)
{
	const struct clar_func *test = suite->tests;
	size_t i, matchlen;
	struct clar_report *report;
	int exact = 0;

	if (!suite->enabled)
		return;

	if (_clar.exit_on_error && _clar.total_errors)
		return;

	if (!_clar.report_errors_only)
		clar_print_onsuite(suite->name, ++_clar.suites_ran);

	_clar.active_suite = suite->name;
	_clar.active_test = NULL;
	CL_TRACE(CL_TRACE__SUITE_BEGIN);

	if (filter) {
		size_t suitelen = strlen(suite->name);
		matchlen = strlen(filter);
		if (matchlen <= suitelen) {
			filter = NULL;
		} else {
			filter += suitelen;
			while (*filter == ':')
				++filter;
			matchlen = strlen(filter);

			if (matchlen && filter[matchlen - 1] == '$') {
				exact = 1;
				matchlen--;
			}
		}
	}


// Source: clar.c
// Lines 294-330
