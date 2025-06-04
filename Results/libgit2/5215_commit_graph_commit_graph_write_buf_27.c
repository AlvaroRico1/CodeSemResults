static int commit_graph_write_buf(const char *buf, size_t size, void *data)
{
	git_str *b = (git_str *)data;
	return git_str_put(b, buf, size);
}


// Source: commit_graph.c
// Lines 937-941
