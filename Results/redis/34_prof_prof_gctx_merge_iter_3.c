prof_gctx_merge_iter(prof_gctx_tree_t *gctxs, prof_gctx_t *gctx, void *opaque) {
	struct prof_gctx_merge_iter_arg_s *arg =
	    (struct prof_gctx_merge_iter_arg_s *)opaque;

	malloc_mutex_lock(arg->tsdn, gctx->lock);
	tctx_tree_iter(&gctx->tctxs, NULL, prof_tctx_merge_iter,
	    (void *)arg->tsdn);
	if (gctx->cnt_summed.curobjs != 0) {
		arg->leak_ngctx++;
	}
	malloc_mutex_unlock(arg->tsdn, gctx->lock);

	return NULL;
}


// Source: prof.c
// Lines 1203-1216
