mode_tree_search_callback(__unused struct client *c, void *data, const char *s,
    __unused int done)
{
	struct mode_tree_data	*mtd = data;

	if (mtd->dead)
		return (0);

	free(mtd->search);
	if (s == NULL || *s == '\0') {
		mtd->search = NULL;
		return (0);
	}
	mtd->search = xstrdup(s);
	mode_tree_search_set(mtd);

	return (0);
}


// Source: mode-tree.c
// Lines 854-871
