prof_tdata_init_impl(tsd_t *tsd, uint64_t thr_uid, uint64_t thr_discrim,
    char *thread_name, bool active) {
	prof_tdata_t *tdata;

	cassert(config_prof);

	/* Initialize an empty cache for this thread. */
	tdata = (prof_tdata_t *)iallocztm(tsd_tsdn(tsd), sizeof(prof_tdata_t),
	    sz_size2index(sizeof(prof_tdata_t)), false, NULL, true,
	    arena_get(TSDN_NULL, 0, true), true);
	if (tdata == NULL) {
		return NULL;
	}

	tdata->lock = prof_tdata_mutex_choose(thr_uid);
	tdata->thr_uid = thr_uid;
	tdata->thr_discrim = thr_discrim;
	tdata->thread_name = thread_name;
	tdata->attached = true;
	tdata->expired = false;
	tdata->tctx_uid_next = 0;

	if (ckh_new(tsd, &tdata->bt2tctx, PROF_CKH_MINITEMS, prof_bt_hash,
	    prof_bt_keycomp)) {
		idalloctm(tsd_tsdn(tsd), tdata, NULL, NULL, true, true);
		return NULL;
	}

	tdata->prng_state = (uint64_t)(uintptr_t)tdata;
	prof_sample_threshold_update(tdata);

	tdata->enq = false;
	tdata->enq_idump = false;
	tdata->enq_gdump = false;

	tdata->dumping = false;
	tdata->active = active;

	malloc_mutex_lock(tsd_tsdn(tsd), &tdatas_mtx);
	tdata_tree_insert(&tdatas, tdata);
	malloc_mutex_unlock(tsd_tsdn(tsd), &tdatas_mtx);

	return tdata;
}


// Source: prof.c
// Lines 1903-1946
