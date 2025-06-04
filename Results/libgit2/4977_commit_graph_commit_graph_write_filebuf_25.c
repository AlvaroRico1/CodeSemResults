static int commit_graph_write_filebuf(const char *buf, size_t size, void *data)
{
	git_filebuf *f = (git_filebuf *)data;
	return git_filebuf_write(f, buf, size);
}


// Source: commit_graph.c
// Lines 1157-1161
