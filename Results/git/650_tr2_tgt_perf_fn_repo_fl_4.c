static void fn_repo_fl(const char *file, int line,
		       const struct repository *repo)
{
	const char *event_name = "def_repo";
	struct strbuf buf_payload = STRBUF_INIT;

	strbuf_addstr(&buf_payload, "worktree:");
	sq_quote_buf_pretty(&buf_payload, repo->worktree);

	perf_io_write_fl(file, line, event_name, repo, NULL, NULL, NULL,
			 &buf_payload);
	strbuf_release(&buf_payload);
}


// Source: tr2_tgt_perf.c
// Lines 450-462
