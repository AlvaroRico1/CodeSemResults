static void init_header(struct ctl_table_header *head,
	struct ctl_table_root *root, struct ctl_table_set *set,
	struct ctl_node *node, struct ctl_table *table)
{
	head->ctl_table = table;
	head->ctl_table_arg = table;
	head->used = 0;
	head->count = 1;
	head->nreg = 1;
	head->unregistering = NULL;
	head->root = root;
	head->set = set;
	head->parent = NULL;
	head->node = node;
	INIT_HLIST_HEAD(&head->inodes);
	if (node) {
		struct ctl_table *entry;
		for (entry = table; entry->procname; entry++, node++)
			node->header = head;
	}


// Source: proc_sysctl.c
// Lines 186-205
