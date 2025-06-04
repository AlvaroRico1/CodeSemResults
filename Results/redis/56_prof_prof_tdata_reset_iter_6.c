prof_tdata_reset_iter(prof_tdata_tree_t *tdatas, prof_tdata_t *tdata,
    void *arg) {
	tsdn_t *tsdn = (tsdn_t *)arg;

	return (prof_tdata_expire(tsdn, tdata) ? tdata : NULL);
}


// Source: prof.c
// Lines 2053-2058
