window_tree_build_pane(struct session *s, struct winlink *wl,
    struct window_pane *wp, void *modedata, struct mode_tree_item *parent)
{
	struct window_tree_modedata	*data = modedata;
	struct window_tree_itemdata	*item;
	char				*name, *text;
	u_int				 idx;

	window_pane_index(wp, &idx);

	item = window_tree_add_item(data);
	item->type = WINDOW_TREE_PANE;
	item->session = s->id;
	item->winlink = wl->idx;
	item->pane = wp->id;

	text = format_single(NULL, data->format, NULL, s, wl, wp);
	xasprintf(&name, "%u", idx);

	mode_tree_add(data->data, parent, item, (uint64_t)wp, name, text, -1);
	free(text);
	free(name);
}


// Source: window-tree.c
// Lines 294-316
