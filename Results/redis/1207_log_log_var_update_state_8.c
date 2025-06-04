log_var_update_state(log_var_t *log_var) {
	const char *log_var_begin = log_var->name;
	const char *log_var_end = log_var->name + strlen(log_var->name);

	/* Pointer to one before the beginning of the current segment. */
	const char *segment_begin = log_var_names;

	/*
	 * If log_init done is false, we haven't parsed the malloc conf yet.  To
	 * avoid log-spew, we default to not displaying anything.
	 */
	if (!atomic_load_b(&log_init_done, ATOMIC_ACQUIRE)) {
		return LOG_INITIALIZED_NOT_ENABLED;
	}

	while (true) {
		const char *segment_end = log_var_extract_segment(
		    segment_begin);
		assert(segment_end < log_var_names + JEMALLOC_LOG_VAR_BUFSIZE);
		if (log_var_matches_segment(segment_begin, segment_end,
		    log_var_begin, log_var_end)) {
			atomic_store_u(&log_var->state, LOG_ENABLED,
			    ATOMIC_RELAXED);
			return LOG_ENABLED;
		}
		if (*segment_end == '\0') {
			/* Hit the end of the segment string with no match. */
			atomic_store_u(&log_var->state,
			    LOG_INITIALIZED_NOT_ENABLED, ATOMIC_RELAXED);
			return LOG_INITIALIZED_NOT_ENABLED;
		}
		/* Otherwise, skip the delimiter and continue. */
		segment_begin = segment_end + 1;
	}


// Source: log.c
// Lines 44-77
