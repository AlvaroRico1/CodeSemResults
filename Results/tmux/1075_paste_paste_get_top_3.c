paste_get_top(const char **name)
{
	struct paste_buffer	*pb;

	pb = RB_MIN(paste_time_tree, &paste_by_time);
	if (pb == NULL)
		return (NULL);
	if (name != NULL)
		*name = pb->name;
	return (pb);
}


// Source: paste.c
// Lines 116-126
