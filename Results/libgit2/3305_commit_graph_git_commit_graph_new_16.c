int git_commit_graph_new(git_commit_graph **cgraph_out, const char *objects_dir, bool open_file)
{
	git_commit_graph *cgraph = NULL;
	int error = 0;

	GIT_ASSERT_ARG(cgraph_out);
	GIT_ASSERT_ARG(objects_dir);

	cgraph = git__calloc(1, sizeof(git_commit_graph));
	GIT_ERROR_CHECK_ALLOC(cgraph);

	error = git_str_joinpath(&cgraph->filename, objects_dir, "info/commit-graph");
	if (error < 0)
		goto error;

	if (open_file) {
		error = git_commit_graph_file_open(&cgraph->file, git_str_cstr(&cgraph->filename));
		if (error < 0)
			goto error;
		cgraph->checked = 1;
	}

	*cgraph_out = cgraph;
	return 0;

error:
	git_commit_graph_free(cgraph);
	return error;
}


// Source: commit_graph.c
// Lines 304-332
