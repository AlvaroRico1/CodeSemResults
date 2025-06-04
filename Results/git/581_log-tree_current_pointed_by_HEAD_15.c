static const struct name_decoration *current_pointed_by_HEAD(const struct name_decoration *decoration)
{
	const struct name_decoration *list, *head = NULL;
	const char *branch_name = NULL;
	int rru_flags;

	/* First find HEAD */
	for (list = decoration; list; list = list->next)
		if (list->type == DECORATION_REF_HEAD) {
			head = list;
			break;
		}
	if (!head)
		return NULL;

	/* Now resolve and find the matching current branch */
	branch_name = resolve_ref_unsafe("HEAD", 0, NULL, &rru_flags);
	if (!branch_name || !(rru_flags & REF_ISSYMREF))
		return NULL;

	if (!starts_with(branch_name, "refs/"))
		return NULL;

	/* OK, do we have that ref in the list? */
	for (list = decoration; list; list = list->next)
		if ((list->type == DECORATION_REF_LOCAL) &&
		    !strcmp(branch_name, list->name)) {
			return list;
		}

	return NULL;
}


// Source: log-tree.c
// Lines 240-271
