int other_head_refs(each_ref_fn fn, void *cb_data)
{
	struct worktree **worktrees, **p;
	struct strbuf refname = STRBUF_INIT;
	int ret = 0;

	worktrees = get_worktrees();
	for (p = worktrees; *p; p++) {
		struct worktree *wt = *p;
		struct object_id oid;
		int flag;

		if (wt->is_current)
			continue;

		strbuf_reset(&refname);
		strbuf_worktree_ref(wt, &refname, "HEAD");
		if (!refs_read_ref_full(get_main_ref_store(the_repository),
					refname.buf,
					RESOLVE_REF_READING,
					&oid, &flag))
			ret = fn(refname.buf, &oid, flag, cb_data);
		if (ret)
			break;
	}


// Source: worktree.c
// Lines 555-579
