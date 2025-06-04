static void free_mailmap_entry(void *p, const char *s)
{
	struct mailmap_entry *me = (struct mailmap_entry *)p;
	debug_mm("mailmap: removing entries for <%s>, with %d sub-entries\n",
		 s, me->namemap.nr);
	debug_mm("mailmap: - simple: '%s' <%s>\n",
		 debug_str(me->name), debug_str(me->email));

	free(me->name);
	free(me->email);

	me->namemap.strdup_strings = 1;
	string_list_clear_func(&me->namemap, free_mailmap_info);
	free(me);
}


// Source: mailmap.c
// Lines 43-57
