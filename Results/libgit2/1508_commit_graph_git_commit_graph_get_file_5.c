int git_commit_graph_get_file(git_commit_graph_file **file_out, git_commit_graph *cgraph)
{
	if (!cgraph->checked) {
		int error = 0;
		git_commit_graph_file *result = NULL;

		/* We only check once, no matter the result. */
		cgraph->checked = 1;

		/* Best effort */
		error = git_commit_graph_file_open(&result, git_str_cstr(&cgraph->filename));

		if (error < 0)
			return error;

		cgraph->file = result;
	}


// Source: commit_graph.c
// Lines 384-400
