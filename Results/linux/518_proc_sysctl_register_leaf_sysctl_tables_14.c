static int register_leaf_sysctl_tables(const char *path, char *pos,
	struct ctl_table_header ***subheader, struct ctl_table_set *set,
	struct ctl_table *table)
{
	struct ctl_table *ctl_table_arg = NULL;
	struct ctl_table *entry, *files;
	int nr_files = 0;
	int nr_dirs = 0;
	int err = -ENOMEM;

	for (entry = table; entry->procname; entry++) {
		if (entry->child)
			nr_dirs++;
		else
			nr_files++;
	}

	files = table;
	/* If there are mixed files and directories we need a new table */
	if (nr_dirs && nr_files) {
		struct ctl_table *new;
		files = kcalloc(nr_files + 1, sizeof(struct ctl_table),
				GFP_KERNEL);
		if (!files)
			goto out;

		ctl_table_arg = files;
		for (new = files, entry = table; entry->procname; entry++) {
			if (entry->child)
				continue;
			*new = *entry;
			new++;
		}
	}

	/* Register everything except a directory full of subdirectories */
	if (nr_files || !nr_dirs) {
		struct ctl_table_header *header;
		header = __register_sysctl_table(set, path, files);
		if (!header) {
			kfree(ctl_table_arg);
			goto out;
		}

		/* Remember if we need to free the file table */
		header->ctl_table_arg = ctl_table_arg;
		**subheader = header;
		(*subheader)++;
	}


// Source: proc_sysctl.c
// Lines 1432-1480
