prof_lookup_global(tsd_t *tsd, prof_bt_t *bt, prof_tdata_t *tdata,
    void **p_btkey, prof_gctx_t **p_gctx, bool *p_new_gctx) {
	union {
		prof_gctx_t	*p;
		void		*v;
	} gctx, tgctx;


// Source: prof.c
// Lines 720-725
