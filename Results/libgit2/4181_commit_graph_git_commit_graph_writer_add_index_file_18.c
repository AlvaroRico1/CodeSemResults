int git_commit_graph_writer_add_index_file(
		git_commit_graph_writer *w,
		git_repository *repo,
		const char *idx_path)
{
	int error;
	struct git_pack_file *p = NULL;
	struct object_entry_cb_state state = {0};
	state.repo = repo;
	state.commits = &w->commits;

	error = git_repository_odb(&state.db, repo);
	if (error < 0)
		goto cleanup;

	error = git_mwindow_get_pack(&p, idx_path);
	if (error < 0)
		goto cleanup;

	error = git_pack_foreach_entry(p, object_entry__cb, &state);
	if (error < 0)
		goto cleanup;

cleanup:
	if (p)
		git_mwindow_put_pack(p);
	git_odb_free(state.db);
	return error;
}


// Source: commit_graph.c
// Lines 700-728
