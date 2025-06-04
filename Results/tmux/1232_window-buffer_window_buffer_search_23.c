window_buffer_search(__unused void *modedata, void *itemdata, const char *ss)
{
	struct window_buffer_itemdata	*item = itemdata;
	struct paste_buffer		*pb;
	const char			*bufdata;
	size_t				 bufsize;

	if ((pb = paste_get_name(item->name)) == NULL)
		return (0);
	if (strstr(item->name, ss) != NULL)
		return (1);
	bufdata = paste_buffer_data(pb, &bufsize);
	return (memmem(bufdata, bufsize, ss, strlen(ss)) != NULL);
}


// Source: window-buffer.c
// Lines 263-276
