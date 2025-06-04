static enum cb_next match_prefix(const struct object_id *oid, void *arg)
{
	struct disambiguate_state *ds = arg;
	/* no need to call match_hash, oidtree_each did prefix match */
	update_candidates(ds, oid);
	return ds->ambiguous ? CB_BREAK : CB_CONTINUE;
}


// Source: object-name.c
// Lines 90-96
