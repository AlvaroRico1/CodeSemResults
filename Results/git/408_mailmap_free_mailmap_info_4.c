static void free_mailmap_info(void *p, const char *s)
{
	struct mailmap_info *mi = (struct mailmap_info *)p;
	debug_mm("mailmap: -- complex: '%s' -> '%s' <%s>\n",
		 s, debug_str(mi->name), debug_str(mi->email));
	free(mi->name);
	free(mi->email);
	free(mi);
}


// Source: mailmap.c
// Lines 33-41
