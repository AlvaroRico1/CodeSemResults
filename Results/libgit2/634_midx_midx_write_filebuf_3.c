static int midx_write_filebuf(const char *buf, size_t size, void *data)
{
	git_filebuf *f = (git_filebuf *)data;
	return git_filebuf_write(f, buf, size);
}


// Source: midx.c
// Lines 849-853
