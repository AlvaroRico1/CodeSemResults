size_t git__linenlen(const char *buffer, size_t buffer_len)
{
	char *nl = memchr(buffer, '\n', buffer_len);
	return nl ? (size_t)(nl - buffer) + 1 : buffer_len;
}


// Source: util.c
// Lines 324-328
