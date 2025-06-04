char *git_str_detach(git_str *buf)
{
	char *data = buf->ptr;

	if (buf->asize == 0 || buf->ptr == git_str__oom)
		return NULL;

	git_str_init(buf, 0);

	return data;
}


// Source: str.c
// Lines 629-639
