menu_free_cb(__unused struct client *c, void *data)
{
	struct menu_data	*md = data;

	if (md->item != NULL)
		cmdq_continue(md->item);

	if (md->cb != NULL)
		md->cb(md->menu, UINT_MAX, KEYC_NONE, md->data);

	screen_free(&md->s);
	menu_free(md->menu);
	free(md);
}


// Source: menu.c
// Lines 203-216
