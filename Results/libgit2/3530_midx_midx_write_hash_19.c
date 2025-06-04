static int midx_write_hash(const char *buf, size_t size, void *data)
{
	struct midx_write_hash_context *ctx = (struct midx_write_hash_context *)data;
	int error;

	error = git_hash_update(ctx->ctx, buf, size);
	if (error < 0)
		return error;

	return ctx->write_cb(buf, size, ctx->cb_data);
}


// Source: midx.c
// Lines 633-643
