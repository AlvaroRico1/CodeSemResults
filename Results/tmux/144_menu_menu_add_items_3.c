menu_add_items(struct menu *menu, const struct menu_item *items,
    struct cmdq_item *qitem, struct client *c, struct cmd_find_state *fs)
{
	const struct menu_item	*loop;

	for (loop = items; loop->name != NULL; loop++)
		menu_add_item(menu, loop, qitem, c, fs);
}


// Source: menu.c
// Lines 44-51
