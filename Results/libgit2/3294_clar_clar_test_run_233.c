clar_test_run(void)
{
	size_t i;
	struct clar_explicit *explicit;

	if (_clar.explicit) {
		for (explicit = _clar.explicit; explicit; explicit = explicit->next)
			clar_run_suite(&_clar_suites[explicit->suite_idx], explicit->filter);
	} else {
		for (i = 0; i < _clar_suite_count; ++i)
			clar_run_suite(&_clar_suites[i], NULL);
	}

	return _clar.total_errors;
}


// Source: clar.c
// Lines 534-548
