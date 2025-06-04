static void print_cmd_by_category(const struct category_description *catdesc,
				  int *longest_p)
{
	struct cmdname_help *cmds;
	int longest = 0;
	int i, nr = 0;
	uint32_t mask = 0;

	for (i = 0; catdesc[i].desc; i++)
		mask |= catdesc[i].category;

	extract_cmds(&cmds, mask);

	for (i = 0; cmds[i].name; i++, nr++) {
		if (longest < strlen(cmds[i].name))
			longest = strlen(cmds[i].name);
	}
	QSORT(cmds, nr, cmd_name_cmp);

	for (i = 0; catdesc[i].desc; i++) {
		uint32_t mask = catdesc[i].category;
		const char *desc = catdesc[i].desc;

		printf("\n%s\n", _(desc));
		print_command_list(cmds, mask, longest);
	}


// Source: help.c
// Lines 104-129
