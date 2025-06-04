int match_pathspec_attrs(struct index_state *istate,
			 const char *name, int namelen,
			 const struct pathspec_item *item)
{
	int i;
	char *to_free = NULL;

	if (name[namelen])
		name = to_free = xmemdupz(name, namelen);

	git_check_attr(istate, name, item->attr_check);

	free(to_free);

	for (i = 0; i < item->attr_match_nr; i++) {
		const char *value;
		int matched;
		enum attr_match_mode match_mode;

		value = item->attr_check->items[i].value;
		match_mode = item->attr_match[i].match_mode;

		if (ATTR_TRUE(value))
			matched = (match_mode == MATCH_SET);
		else if (ATTR_FALSE(value))
			matched = (match_mode == MATCH_UNSET);
		else if (ATTR_UNSET(value))
			matched = (match_mode == MATCH_UNSPECIFIED);
		else
			matched = (match_mode == MATCH_VALUE &&
				   !strcmp(item->attr_match[i].value, value));
		if (!matched)
			return 0;
	}

	return 1;
}


// Source: pathspec.c
// Lines 725-761
