format_cb_window_visible_layout(struct format_tree *ft)
{
	struct window	*w = ft->w;

	if (w == NULL)
		return (NULL);

	return (layout_dump(w->layout_root));
}


// Source: format.c
// Lines 780-788
