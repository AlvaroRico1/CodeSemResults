tcaches_elm_remove(tsd_t *tsd, tcaches_t *elm) {
	malloc_mutex_assert_owner(tsd_tsdn(tsd), &tcaches_mtx);

	if (elm->tcache == NULL) {
		return NULL;
	}
	tcache_t *tcache = elm->tcache;
	elm->tcache = NULL;
	return tcache;
}


// Source: tcache.c
// Lines 617-626
