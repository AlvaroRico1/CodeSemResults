static int sysctl_follow_link(struct ctl_table_header **phead,
	struct ctl_table **pentry)
{
	struct ctl_table_header *head;
	struct ctl_table_root *root;
	struct ctl_table_set *set;
	struct ctl_table *entry;
	struct ctl_dir *dir;
	int ret;

	ret = 0;
	spin_lock(&sysctl_lock);
	root = (*pentry)->data;
	set = lookup_header_set(root);
	dir = xlate_dir(set, (*phead)->parent);
	if (IS_ERR(dir))
		ret = PTR_ERR(dir);
	else {
		const char *procname = (*pentry)->procname;
		head = NULL;
		entry = find_entry(&head, dir, procname, strlen(procname));
		ret = -ENOENT;
		if (entry && use_table(head)) {
			unuse_table(*phead);
			*phead = head;
			*pentry = entry;
			ret = 0;
		}
	}


// Source: proc_sysctl.c
// Lines 1065-1093
