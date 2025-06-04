tsd_cleanup(void *arg) {
	tsd_t *tsd = (tsd_t *)arg;

	switch (tsd->state) {
	case tsd_state_uninitialized:
		/* Do nothing. */
		break;
	case tsd_state_minimal_initialized:
		/* This implies the thread only did free() in its life time. */
		/* Fall through. */
	case tsd_state_reincarnated:
		/*
		 * Reincarnated means another destructor deallocated memory
		 * after the destructor was called.  Cleanup isn't required but
		 * is still called for testing and completeness.
		 */
		assert_tsd_data_cleanup_done(tsd);
		/* Fall through. */
	case tsd_state_nominal:
	case tsd_state_nominal_slow:
		tsd_do_data_cleanup(tsd);
		tsd->state = tsd_state_purgatory;
		tsd_set(tsd);
		break;
	case tsd_state_purgatory:
		/*
		 * The previous time this destructor was called, we set the
		 * state to tsd_state_purgatory so that other destructors
		 * wouldn't cause re-creation of the tsd.  This time, do
		 * nothing, and do not request another callback.
		 */
		break;
	default:
		not_reached();
	}
#ifdef JEMALLOC_JET
	test_callback_t test_callback = *tsd_test_callbackp_get_unsafe(tsd);
	int *data = tsd_test_datap_get_unsafe(tsd);
	if (test_callback != NULL) {
		test_callback(data);
	}
#endif
}


// Source: tsd.c
// Lines 214-256
