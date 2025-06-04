static int show_ambiguous_object(const struct object_id *oid, void *data)
{
	const struct disambiguate_state *ds = data;
	struct strbuf desc = STRBUF_INIT;
	int type;

	if (ds->fn && !ds->fn(ds->repo, oid, ds->cb_data))
		return 0;

	type = oid_object_info(ds->repo, oid, NULL);
	if (type == OBJ_COMMIT) {
		struct commit *commit = lookup_commit(ds->repo, oid);
		if (commit) {
			struct pretty_print_context pp = {0};
			pp.date_mode.type = DATE_SHORT;
			format_commit_message(commit, " %ad - %s", &desc, &pp);
		}
	} else if (type == OBJ_TAG) {
		struct tag *tag = lookup_tag(ds->repo, oid);
		if (!parse_tag(tag) && tag->tag)
			strbuf_addf(&desc, " %s", tag->tag);
	}

	advise("  %s %s%s",
	       repo_find_unique_abbrev(ds->repo, oid, DEFAULT_ABBREV),
	       type_name(type) ? type_name(type) : "unknown type",
	       desc.buf);

	strbuf_release(&desc);
	return 0;
}


// Source: object-name.c
// Lines 354-384
