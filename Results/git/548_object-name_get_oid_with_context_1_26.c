static enum get_oid_result get_oid_with_context_1(struct repository *repo,
				  const char *name,
				  unsigned flags,
				  const char *prefix,
				  struct object_id *oid,
				  struct object_context *oc)
{
	int ret, bracket_depth;
	int namelen = strlen(name);
	const char *cp;
	int only_to_die = flags & GET_OID_ONLY_TO_DIE;

	if (only_to_die)
		flags |= GET_OID_QUIETLY;

	memset(oc, 0, sizeof(*oc));
	oc->mode = S_IFINVALID;
	strbuf_init(&oc->symlink_path, 0);
	ret = get_oid_1(repo, name, namelen, oid, flags);
	if (!ret)
		return ret;
	/*
	 * tree:path --> object name of path in tree
	 * :path -> object name of absolute path in index
	 * :./path -> object name of path relative to cwd in index
	 * :[0-3]:path -> object name of path in index at stage
	 * :/foo -> recent commit matching foo
	 */
	if (name[0] == ':') {
		int stage = 0;
		const struct cache_entry *ce;
		char *new_path = NULL;
		int pos;
		if (!only_to_die && namelen > 2 && name[1] == '/') {
			struct handle_one_ref_cb cb;
			struct commit_list *list = NULL;

			cb.repo = repo;
			cb.list = &list;
			refs_for_each_ref(get_main_ref_store(repo), handle_one_ref, &cb);
			refs_head_ref(get_main_ref_store(repo), handle_one_ref, &cb);
			commit_list_sort_by_date(&list);
			return get_oid_oneline(repo, name + 2, oid, list);
		}
		if (namelen < 3 ||
		    name[2] != ':' ||
		    name[1] < '0' || '3' < name[1])
			cp = name + 1;
		else {
			stage = name[1] - '0';
			cp = name + 3;
		}
		new_path = resolve_relative_path(repo, cp);
		if (!new_path) {
			namelen = namelen - (cp - name);
		} else {
			cp = new_path;
			namelen = strlen(cp);
		}

		if (flags & GET_OID_RECORD_PATH)
			oc->path = xstrdup(cp);

		if (!repo->index || !repo->index->cache)
			repo_read_index(repo);
		pos = index_name_pos(repo->index, cp, namelen);
		if (pos < 0)
			pos = -pos - 1;
		while (pos < repo->index->cache_nr) {
			ce = repo->index->cache[pos];
			if (ce_namelen(ce) != namelen ||
			    memcmp(ce->name, cp, namelen))
				break;
			if (ce_stage(ce) == stage) {
				oidcpy(oid, &ce->oid);
				oc->mode = ce->ce_mode;
				free(new_path);
				return 0;
			}
			pos++;
		}
		if (only_to_die && name[1] && name[1] != '/')
			diagnose_invalid_index_path(repo, stage, prefix, cp);
		free(new_path);
		return -1;
	}


// Source: object-name.c
// Lines 1786-1871
