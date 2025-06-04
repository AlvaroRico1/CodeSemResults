window_client_build(void *modedata, struct mode_tree_sort_criteria *sort_crit,
    __unused uint64_t *tag, const char *filter)
{
	struct window_client_modedata	*data = modedata;
	struct window_client_itemdata	*item;
	u_int				 i;
	struct client			*c;
	char				*text, *cp;

	for (i = 0; i < data->item_size; i++)
		window_client_free_item(data->item_list[i]);
	free(data->item_list);
	data->item_list = NULL;
	data->item_size = 0;

	TAILQ_FOREACH(c, &clients, entry) {
		if (c->session == NULL || (c->flags & CLIENT_UNATTACHEDFLAGS))
			continue;

		item = window_client_add_item(data);
		item->c = c;

		c->references++;
	}

	window_client_sort = sort_crit;
	qsort(data->item_list, data->item_size, sizeof *data->item_list,
	    window_client_cmp);

	for (i = 0; i < data->item_size; i++) {
		item = data->item_list[i];
		c = item->c;

		if (filter != NULL) {
			cp = format_single(NULL, filter, c, NULL, NULL, NULL);
			if (!format_true(cp)) {
				free(cp);
				continue;
			}
			free(cp);
		}

		text = format_single(NULL, data->format, c, NULL, NULL, NULL);
		mode_tree_add(data->data, NULL, item, (uint64_t)c, c->name,
		    text, -1);
		free(text);
	}
}


// Source: window-client.c
// Lines 167-214
