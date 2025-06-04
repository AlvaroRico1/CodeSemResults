static void put_links(struct ctl_table_header *header)
{
	struct ctl_table_set *root_set = &sysctl_table_root.default_set;
	struct ctl_table_root *root = header->root;
	struct ctl_dir *parent = header->parent;
	struct ctl_dir *core_parent;
	struct ctl_table *entry;

	if (header->set == root_set)
		return;

	core_parent = xlate_dir(root_set, parent);
	if (IS_ERR(core_parent))
		return;

	for (entry = header->ctl_table; entry->procname; entry++) {
		struct ctl_table_header *link_head;
		struct ctl_table *link;
		const char *name = entry->procname;

		link = find_entry(&link_head, core_parent, name, strlen(name));
		if (link &&
		    ((S_ISDIR(link->mode) && S_ISDIR(entry->mode)) ||
		     (S_ISLNK(link->mode) && (link->data == root)))) {
			drop_sysctl_table(link_head);
		}
		else {
			pr_err("sysctl link missing during unregister: ");
			sysctl_print_dir(parent);
			pr_cont("/%s\n", name);
		}
	}


// Source: proc_sysctl.c
// Lines 1613-1644
