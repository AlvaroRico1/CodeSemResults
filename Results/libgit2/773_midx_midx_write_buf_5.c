static int midx_write_buf(const char *buf, size_t size, void *data)
{
	git_str *b = (git_str *)data;
	return git_str_put(b, buf, size);
}


// Source: midx.c
// Lines 621-625
