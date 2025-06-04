		void		*v;
	} ret;
	prof_tdata_t *tdata;
	bool not_found;

	cassert(config_prof);

	tdata = prof_tdata_get(tsd, false);
	if (tdata == NULL) {
		return NULL;
	}

	malloc_mutex_lock(tsd_tsdn(tsd), tdata->lock);
	not_found = ckh_search(&tdata->bt2tctx, bt, NULL, &ret.v);
	if (!not_found) { /* Note double negative! */
		ret.p->prepared = true;
	}
	malloc_mutex_unlock(tsd_tsdn(tsd), tdata->lock);
	if (not_found) {
		void *btkey;
		prof_gctx_t *gctx;
		bool new_gctx, error;

		/*
		 * This thread's cache lacks bt.  Look for it in the global
		 * cache.
		 */
		if (prof_lookup_global(tsd, bt, tdata, &btkey, &gctx,
		    &new_gctx)) {
			return NULL;
		}

		/* Link a prof_tctx_t into gctx for this thread. */
		ret.v = iallocztm(tsd_tsdn(tsd), sizeof(prof_tctx_t),
		    sz_size2index(sizeof(prof_tctx_t)), false, NULL, true,
		    arena_ichoose(tsd, NULL), true);
		if (ret.p == NULL) {
			if (new_gctx) {
				prof_gctx_try_destroy(tsd, tdata, gctx, tdata);
			}
			return NULL;
		}
		ret.p->tdata = tdata;
		ret.p->thr_uid = tdata->thr_uid;
		ret.p->thr_discrim = tdata->thr_discrim;
		memset(&ret.p->cnts, 0, sizeof(prof_cnt_t));
		ret.p->gctx = gctx;
		ret.p->tctx_uid = tdata->tctx_uid_next++;
		ret.p->prepared = true;
		ret.p->state = prof_tctx_state_initializing;
		malloc_mutex_lock(tsd_tsdn(tsd), tdata->lock);
		error = ckh_insert(tsd, &tdata->bt2tctx, btkey, ret.v);
		malloc_mutex_unlock(tsd_tsdn(tsd), tdata->lock);
		if (error) {
			if (new_gctx) {
				prof_gctx_try_destroy(tsd, tdata, gctx, tdata);
			}
			idalloctm(tsd_tsdn(tsd), ret.v, NULL, NULL, true, true);
			return NULL;
		}
		malloc_mutex_lock(tsd_tsdn(tsd), gctx->lock);
		ret.p->state = prof_tctx_state_nominal;
		tctx_tree_insert(&gctx->tctxs, ret.p);
		gctx->nlimbo--;
		malloc_mutex_unlock(tsd_tsdn(tsd), gctx->lock);
	}

	return ret.p;
}


// Source: prof.c
// Lines 788-856
