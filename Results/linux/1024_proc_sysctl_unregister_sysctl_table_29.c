void unregister_sysctl_table(struct ctl_table_header * header)
{
	int nr_subheaders;
	might_sleep();

	if (header == NULL)
		return;

	nr_subheaders = count_subheaders(header->ctl_table_arg);
	if (unlikely(nr_subheaders > 1)) {
		struct ctl_table_header **subheaders;
		int i;

		subheaders = (struct ctl_table_header **)(header + 1);
		for (i = nr_subheaders -1; i >= 0; i--) {
			struct ctl_table_header *subh = subheaders[i];
			struct ctl_table *table = subh->ctl_table_arg;
			unregister_sysctl_table(subh);
			kfree(table);
		}
		kfree(header);
		return;
	}


// Source: proc_sysctl.c
// Lines 1673-1695
