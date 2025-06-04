int map_user(struct string_list *map,
	     const char **email, size_t *emaillen,
	     const char **name, size_t *namelen)
{
	struct string_list_item *item;
	struct mailmap_entry *me;

	debug_mm("map_user: map '%.*s' <%.*s>\n",
		 (int)*namelen, debug_str(*name),
		 (int)*emaillen, debug_str(*email));

	item = lookup_prefix(map, *email, *emaillen);
	if (item != NULL) {
		me = (struct mailmap_entry *)item->util;
		if (me->namemap.nr) {
			/*
			 * The item has multiple items, so we'll look up on
			 * name too. If the name is not found, we choose the
			 * simple entry.
			 */
			struct string_list_item *subitem;
			subitem = lookup_prefix(&me->namemap, *name, *namelen);
			if (subitem)
				item = subitem;
		}


// Source: mailmap.c
// Lines 310-334
