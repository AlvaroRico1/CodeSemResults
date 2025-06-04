prof_tctx_dump_iter(prof_tctx_tree_t *tctxs, prof_tctx_t *tctx, void *opaque) {
	struct prof_tctx_dump_iter_arg_s *arg =
	    (struct prof_tctx_dump_iter_arg_s *)opaque;

	malloc_mutex_assert_owner(arg->tsdn, tctx->gctx->lock);

	switch (tctx->state) {
	case prof_tctx_state_initializing:
	case prof_tctx_state_nominal:
		/* Not captured by this dump. */
		break;
	case prof_tctx_state_dumping:
	case prof_tctx_state_purgatory:
		if (prof_dump_printf(arg->propagate_err,
		    "  t%"FMTu64": %"FMTu64": %"FMTu64" [%"FMTu64": "
		    "%"FMTu64"]\n", tctx->thr_uid, tctx->dump_cnts.curobjs,
		    tctx->dump_cnts.curbytes, tctx->dump_cnts.accumobjs,
		    tctx->dump_cnts.accumbytes)) {
			return tctx;
		}
		break;
	default:
		not_reached();
	}
	return NULL;
}


// Source: prof.c
// Lines 1125-1150
