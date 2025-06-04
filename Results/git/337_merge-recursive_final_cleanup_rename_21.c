static void final_cleanup_rename(struct string_list *rename)
{
	const struct rename *re;
	int i;

	if (rename == NULL)
		return;

	for (i = 0; i < rename->nr; i++) {
		re = rename->items[i].util;
		diff_free_filepair(re->pair);
	}
	string_list_clear(rename, 1);
	free(rename);
}


// Source: merge-recursive.c
// Lines 2992-3006
