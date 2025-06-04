static int sysctl_perm(struct ctl_table_header *head, struct ctl_table *table, int op)
{
	struct ctl_table_root *root = head->root;
	int mode;

	if (root->permissions)
		mode = root->permissions(head, table);
	else
		mode = table->mode;

	return test_perm(mode, op);
}


// Source: proc_sysctl.c
// Lines 450-461
