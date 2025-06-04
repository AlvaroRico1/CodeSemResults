static int ident_remove_id(
	git_str *to, const git_str *from)
{
	const char *id_start, *id_end, *from_end = from->ptr + from->size;
	size_t need_size;

	if (ident_find_id(&id_start, &id_end, from->ptr, from->size) < 0)
		return GIT_PASSTHROUGH;

	need_size = (size_t)(id_start - from->ptr) +
		4 /* "$Id$" */ + (size_t)(from_end - id_end);

	if (git_str_grow(to, need_size) < 0)
		return -1;

	git_str_set(to, from->ptr, (size_t)(id_start - from->ptr));
	git_str_put(to, "$Id$", 4);
	git_str_put(to, id_end, (size_t)(from_end - id_end));

	return git_str_oom(to) ? -1 : 0;
}


// Source: ident.c
// Lines 75-95
