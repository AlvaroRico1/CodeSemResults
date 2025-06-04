prof_tdata_merge_iter(prof_tdata_tree_t *tdatas, prof_tdata_t *tdata,
    void *opaque) {
	struct prof_tdata_merge_iter_arg_s *arg =
	    (struct prof_tdata_merge_iter_arg_s *)opaque;

	malloc_mutex_lock(arg->tsdn, tdata->lock);
	if (!tdata->expired) {
		size_t tabind;
		union {
			prof_tctx_t	*p;
			void		*v;
		} tctx;

		tdata->dumping = true;
		memset(&tdata->cnt_summed, 0, sizeof(prof_cnt_t));
		for (tabind = 0; !ckh_iter(&tdata->bt2tctx, &tabind, NULL,
		    &tctx.v);) {
			prof_tctx_merge_tdata(arg->tsdn, tctx.p, tdata);
		}

		arg->cnt_all.curobjs += tdata->cnt_summed.curobjs;
		arg->cnt_all.curbytes += tdata->cnt_summed.curbytes;
		if (opt_prof_accum) {
			arg->cnt_all.accumobjs += tdata->cnt_summed.accumobjs;
			arg->cnt_all.accumbytes += tdata->cnt_summed.accumbytes;
		}
	} else {
		tdata->dumping = false;
	}
	malloc_mutex_unlock(arg->tsdn, tdata->lock);

	return NULL;
}


// Source: prof.c
// Lines 1270-1302
