static int clear_ce_flags_dir(struct index_state *istate,
			      struct cache_entry **cache, int nr,
			      struct strbuf *prefix,
			      char *basename,
			      int select_mask, int clear_mask,
			      struct pattern_list *pl,
			      enum pattern_match_result default_match,
			      int progress_nr)
{
	struct cache_entry **cache_end;
	int dtype = DT_DIR;
	int rc;
	enum pattern_match_result ret, orig_ret;
	orig_ret = path_matches_pattern_list(prefix->buf, prefix->len,
					     basename, &dtype, pl, istate);

	strbuf_addch(prefix, '/');

	/* If undecided, use matching result of parent dir in defval */
	if (orig_ret == UNDECIDED)
		ret = default_match;
	else
		ret = orig_ret;

	for (cache_end = cache; cache_end != cache + nr; cache_end++) {
		struct cache_entry *ce = *cache_end;
		if (strncmp(ce->name, prefix->buf, prefix->len))
			break;
	}

	if (pl->use_cone_patterns && orig_ret == MATCHED_RECURSIVE) {
		struct cache_entry **ce = cache;
		rc = cache_end - cache;

		while (ce < cache_end) {
			(*ce)->ce_flags &= ~clear_mask;
			ce++;
		}
	} else if (pl->use_cone_patterns && orig_ret == NOT_MATCHED) {
		rc = cache_end - cache;
	} else {
		rc = clear_ce_flags_1(istate, cache, cache_end - cache,
				      prefix,
				      select_mask, clear_mask,
				      pl, ret,
				      progress_nr);
	}

	strbuf_setlen(prefix, prefix->len - 1);
	return rc;
}


// Source: unpack-trees.c
// Lines 1455-1505
