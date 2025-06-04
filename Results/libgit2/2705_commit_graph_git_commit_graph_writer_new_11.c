int git_commit_graph_writer_new(git_commit_graph_writer **out, const char *objects_info_dir)
{
	git_commit_graph_writer *w = git__calloc(1, sizeof(git_commit_graph_writer));
	GIT_ERROR_CHECK_ALLOC(w);

	if (git_str_sets(&w->objects_info_dir, objects_info_dir) < 0) {
		git__free(w);
		return -1;
	}

	if (git_vector_init(&w->commits, 0, packed_commit__cmp) < 0) {
		git_str_dispose(&w->objects_info_dir);
		git__free(w);
		return -1;
	}

	*out = w;
	return 0;
}


// Source: commit_graph.c
// Lines 626-644
