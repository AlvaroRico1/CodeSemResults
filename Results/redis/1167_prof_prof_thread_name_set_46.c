prof_thread_name_set(tsd_t *tsd, const char *thread_name) {
	prof_tdata_t *tdata;
	unsigned i;
	char *s;

	tdata = prof_tdata_get(tsd, true);
	if (tdata == NULL) {
		return EAGAIN;
	}

	/* Validate input. */
	if (thread_name == NULL) {
		return EFAULT;
	}
	for (i = 0; thread_name[i] != '\0'; i++) {
		char c = thread_name[i];
		if (!isgraph(c) && !isblank(c)) {
			return EFAULT;
		}
	}

	s = prof_thread_name_alloc(tsd_tsdn(tsd), thread_name);
	if (s == NULL) {
		return EAGAIN;
	}

	if (tdata->thread_name != NULL) {
		idalloctm(tsd_tsdn(tsd), tdata->thread_name, NULL, NULL, true,
		    true);
		tdata->thread_name = NULL;
	}
	if (strlen(s) > 0) {
		tdata->thread_name = s;
	}
	return 0;
}


// Source: prof.c
// Lines 2157-2192
