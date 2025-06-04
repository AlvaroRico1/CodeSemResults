static void files_reflog_path_other_worktrees(struct files_ref_store *refs,
					      struct strbuf *sb,
					      const char *refname)
{
	const char *real_ref;
	const char *worktree_name;
	int length;

	if (parse_worktree_ref(refname, &worktree_name, &length, &real_ref))
		BUG("refname %s is not a other-worktree ref", refname);

	if (worktree_name)
		strbuf_addf(sb, "%s/worktrees/%.*s/logs/%s", refs->gitcommondir,
			    length, worktree_name, real_ref);
	else
		strbuf_addf(sb, "%s/logs/%s", refs->gitcommondir,
			    real_ref);
}


// Source: files-backend.c
// Lines 144-161
