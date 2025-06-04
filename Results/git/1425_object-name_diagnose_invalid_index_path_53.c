static void diagnose_invalid_index_path(struct repository *r,
					int stage,
					const char *prefix,
					const char *filename)
{
	struct index_state *istate = r->index;
	const struct cache_entry *ce;
	int pos;
	unsigned namelen = strlen(filename);
	struct strbuf fullname = STRBUF_INIT;

	if (!prefix)
		prefix = "";

	/* Wrong stage number? */
	pos = index_name_pos(istate, filename, namelen);
	if (pos < 0)
		pos = -pos - 1;
	if (pos < istate->cache_nr) {
		ce = istate->cache[pos];
		if (ce_namelen(ce) == namelen &&
		    !memcmp(ce->name, filename, namelen))
			die(_("path '%s' is in the index, but not at stage %d\n"
			    "hint: Did you mean ':%d:%s'?"),
			    filename, stage,
			    ce_stage(ce), filename);
	}

	/* Confusion between relative and absolute filenames? */
	strbuf_addstr(&fullname, prefix);
	strbuf_addstr(&fullname, filename);
	pos = index_name_pos(istate, fullname.buf, fullname.len);
	if (pos < 0)
		pos = -pos - 1;
	if (pos < istate->cache_nr) {
		ce = istate->cache[pos];
		if (ce_namelen(ce) == fullname.len &&
		    !memcmp(ce->name, fullname.buf, fullname.len))
			die(_("path '%s' is in the index, but not '%s'\n"
			    "hint: Did you mean ':%d:%s' aka ':%d:./%s'?"),
			    fullname.buf, filename,
			    ce_stage(ce), fullname.buf,
			    ce_stage(ce), filename);
	}

	if (repo_file_exists(r, filename))
		die(_("path '%s' exists on disk, but not in the index"), filename);
	if (is_missing_file_error(errno))
		die(_("path '%s' does not exist (neither on disk nor in the index)"),
		    filename);

	strbuf_release(&fullname);
}


// Source: object-name.c
// Lines 1717-1769
