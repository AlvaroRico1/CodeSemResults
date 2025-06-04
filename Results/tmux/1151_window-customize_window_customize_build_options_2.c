window_customize_build_options(struct window_customize_modedata *data,
    const char *title, uint64_t tag,
    enum window_customize_scope scope0, struct options *oo0,
    enum window_customize_scope scope1, struct options *oo1,
    enum window_customize_scope scope2, struct options *oo2,
    struct format_tree *ft, const char *filter, struct cmd_find_state *fs)
{
	struct mode_tree_item		 *top;
	struct options_entry		 *o = NULL, *loop;
	const char			**list = NULL, *name;
	u_int				  size = 0, i;
	enum window_customize_scope	  scope;

	top = mode_tree_add(data->data, NULL, NULL, tag, title, NULL, 0);
	mode_tree_no_tag(top);

	/*
	 * We get the options from the first tree, but build it using the
	 * values from the other two. Any tree can have user options so we need
	 * to build a separate list of them.
	 */

	window_customize_find_user_options(oo0, &list, &size);
	if (oo1 != NULL)
		window_customize_find_user_options(oo1, &list, &size);
	if (oo2 != NULL)
		window_customize_find_user_options(oo2, &list, &size);

	for (i = 0; i < size; i++) {
		if (oo2 != NULL)
			o = options_get(oo0, list[i]);
		if (o == NULL && oo1 != NULL)
			o = options_get(oo1, list[i]);
		if (o == NULL)
			o = options_get(oo2, list[i]);
		if (options_owner(o) == oo2)
			scope = scope2;
		else if (options_owner(o) == oo1)
			scope = scope1;
		else
			scope = scope0;
		window_customize_build_option(data, top, scope, o, ft, filter,
		    fs);
	}
	free(list);

	loop = options_first(oo0);
	while (loop != NULL) {
		name = options_name(loop);
		if (*name == '@') {
			loop = options_next(loop);
			continue;
		}
		if (oo2 != NULL)
			o = options_get(oo2, name);
		else if (oo1 != NULL)
			o = options_get(oo1, name);
		else
			o = loop;
		if (options_owner(o) == oo2)
			scope = scope2;
		else if (options_owner(o) == oo1)
			scope = scope1;
		else
			scope = scope0;
		window_customize_build_option(data, top, scope, o, ft, filter,
		    fs);
		loop = options_next(loop);
	}
}


// Source: window-customize.c
// Lines 371-440
