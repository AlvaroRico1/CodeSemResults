static struct ctl_table *lookup_entry(struct ctl_table_header **phead,
				      struct ctl_dir *dir,
				      const char *name, int namelen)
{
	struct ctl_table_header *head;
	struct ctl_table *entry;

	spin_lock(&sysctl_lock);
	entry = find_entry(&head, dir, name, namelen);
	if (entry && use_table(head))
		*phead = head;
	else
		entry = NULL;
	spin_unlock(&sysctl_lock);
	return entry;
}


// Source: proc_sysctl.c
// Lines 367-382
