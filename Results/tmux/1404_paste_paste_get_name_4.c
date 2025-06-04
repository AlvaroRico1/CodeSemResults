paste_get_name(const char *name)
{
	struct paste_buffer	pbfind;

	if (name == NULL || *name == '\0')
		return (NULL);

	pbfind.name = (char *)name;
	return (RB_FIND(paste_name_tree, &paste_by_name, &pbfind));
}


// Source: paste.c
// Lines 130-139
