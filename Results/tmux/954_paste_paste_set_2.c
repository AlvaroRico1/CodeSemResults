paste_set(char *data, size_t size, const char *name, char **cause)
{
	struct paste_buffer	*pb, *old;

	if (cause != NULL)
		*cause = NULL;

	if (size == 0) {
		free(data);
		return (0);
	}
	if (name == NULL) {
		paste_add(NULL, data, size);
		return (0);
	}

	if (*name == '\0') {
		if (cause != NULL)
			*cause = xstrdup("empty buffer name");
		return (-1);
	}

	pb = xmalloc(sizeof *pb);

	pb->name = xstrdup(name);

	pb->data = data;
	pb->size = size;

	pb->automatic = 0;
	pb->order = paste_next_order++;

	pb->created = time(NULL);

	if ((old = paste_get_name(name)) != NULL)
		paste_free(old);

	RB_INSERT(paste_name_tree, &paste_by_name, pb);
	RB_INSERT(paste_time_tree, &paste_by_time, pb);

	return (0);
}


// Source: paste.c
// Lines 256-297
