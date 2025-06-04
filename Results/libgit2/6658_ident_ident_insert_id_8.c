static int ident_insert_id(
	git_str *to, const git_str *from, const git_filter_source *src)
{
	char oid[GIT_OID_HEXSZ+1];
	const char *id_start, *id_end, *from_end = from->ptr + from->size;
	size_t need_size;

	/* replace $Id$ with blob id */

	if (!git_filter_source_id(src))
		return GIT_PASSTHROUGH;

	git_oid_tostr(oid, sizeof(oid), git_filter_source_id(src));

	if (ident_find_id(&id_start, &id_end, from->ptr, from->size) < 0)
		return GIT_PASSTHROUGH;

	need_size = (size_t)(id_start - from->ptr) +
		5 /* "$Id: " */ + GIT_OID_HEXSZ + 2 /* " $" */ +
		(size_t)(from_end - id_end);

	if (git_str_grow(to, need_size) < 0)
		return -1;

	git_str_set(to, from->ptr, (size_t)(id_start - from->ptr));
	git_str_put(to, "$Id: ", 5);
	git_str_put(to, oid, GIT_OID_HEXSZ);
	git_str_put(to, " $", 2);
	git_str_put(to, id_end, (size_t)(from_end - id_end));

	return git_str_oom(to) ? -1 : 0;
}


// Source: ident.c
// Lines 42-73
