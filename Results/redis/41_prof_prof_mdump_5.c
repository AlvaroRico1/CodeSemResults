prof_mdump(tsd_t *tsd, const char *filename) {
	cassert(config_prof);
	assert(tsd_reentrancy_level_get(tsd) == 0);

	if (!opt_prof || !prof_booted) {
		return true;
	}
	char filename_buf[DUMP_FILENAME_BUFSIZE];
	if (filename == NULL) {
		/* No filename specified, so automatically generate one. */
		if (opt_prof_prefix[0] == '\0') {
			return true;
		}
		malloc_mutex_lock(tsd_tsdn(tsd), &prof_dump_seq_mtx);
		prof_dump_filename(filename_buf, 'm', prof_dump_mseq);
		prof_dump_mseq++;
		malloc_mutex_unlock(tsd_tsdn(tsd), &prof_dump_seq_mtx);
		filename = filename_buf;
	}
	return prof_dump(tsd, true, filename, false);
}


// Source: prof.c
// Lines 1812-1832
