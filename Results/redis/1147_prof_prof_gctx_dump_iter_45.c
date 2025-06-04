prof_gctx_dump_iter(prof_gctx_tree_t *gctxs, prof_gctx_t *gctx, void *opaque) {
	prof_gctx_t *ret;
	struct prof_gctx_dump_iter_arg_s *arg =
	    (struct prof_gctx_dump_iter_arg_s *)opaque;

	malloc_mutex_lock(arg->tsdn, gctx->lock);

	if (prof_dump_gctx(arg->tsdn, arg->propagate_err, gctx, &gctx->bt,
	    gctxs)) {
		ret = gctx;
		goto label_return;
	}

	ret = NULL;
label_return:
	malloc_mutex_unlock(arg->tsdn, gctx->lock);
	return ret;
}


// Source: prof.c
// Lines 1532-1549
