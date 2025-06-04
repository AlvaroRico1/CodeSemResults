static int add_cacheinfo(struct merge_options *opt,
			 const struct diff_filespec *blob,
			 const char *path, int stage, int refresh, int options)
{
	struct index_state *istate = opt->repo->index;
	struct cache_entry *ce;
	int ret;

	ce = make_cache_entry(istate, blob->mode, &blob->oid, path, stage, 0);
	if (!ce)
		return err(opt, _("add_cacheinfo failed for path '%s'; merge aborting."), path);

	ret = add_index_entry(istate, ce, options);
	if (refresh) {
		struct cache_entry *nce;

		nce = refresh_cache_entry(istate, ce,
					  CE_MATCH_REFRESH | CE_MATCH_IGNORE_MISSING);
		if (!nce)
			return err(opt, _("add_cacheinfo failed to refresh for path '%s'; merge aborting."), path);
		if (nce != ce)
			ret = add_index_entry(istate, nce, options);
	}
	return ret;
}


// Source: merge-recursive.c
// Lines 363-387
