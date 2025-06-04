static int commit_graph_write_hash(const char *buf, size_t size, void *data)
{
	struct commit_graph_write_hash_context *ctx = data;
	int error;

	error = git_hash_update(ctx->ctx, buf, size);
	if (error < 0)
		return error;

	return ctx->write_cb(buf, size, ctx->cb_data);
}


// Source: commit_graph.c
// Lines 949-959
