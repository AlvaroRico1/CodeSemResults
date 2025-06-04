prof_tctx_merge_iter(prof_tctx_tree_t *tctxs, prof_tctx_t *tctx, void *arg) {
	tsdn_t *tsdn = (tsdn_t *)arg;

	malloc_mutex_assert_owner(tsdn, tctx->gctx->lock);

	switch (tctx->state) {
	case prof_tctx_state_nominal:
		/* New since dumping started; ignore. */
		break;
	case prof_tctx_state_dumping:
	case prof_tctx_state_purgatory:
		prof_tctx_merge_gctx(tsdn, tctx, tctx->gctx);
		break;
	default:
		not_reached();
	}

	return NULL;
}


// Source: prof.c
// Lines 1099-1117
