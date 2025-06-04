prof_tctx_finish_iter(prof_tctx_tree_t *tctxs, prof_tctx_t *tctx, void *arg) {
	tsdn_t *tsdn = (tsdn_t *)arg;
	prof_tctx_t *ret;

	malloc_mutex_assert_owner(tsdn, tctx->gctx->lock);

	switch (tctx->state) {
	case prof_tctx_state_nominal:
		/* New since dumping started; ignore. */
		break;
	case prof_tctx_state_dumping:
		tctx->state = prof_tctx_state_nominal;
		break;
	case prof_tctx_state_purgatory:
		ret = tctx;
		goto label_return;
	default:
		not_reached();
	}

	ret = NULL;
label_return:
	return ret;
}


// Source: prof.c
// Lines 1153-1176
