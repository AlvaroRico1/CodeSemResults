format_cb_window_layout(struct format_tree *ft)
{
	struct window	*w = ft->w;

	if (w == NULL)
		return (NULL);

	if (w->saved_layout_root != NULL)
		return (layout_dump(w->saved_layout_root));
	return (layout_dump(w->layout_root));
}


// Source: format.c
// Lines 766-776
