static struct ctl_dir *new_dir(struct ctl_table_set *set,
			       const char *name, int namelen)
{
	struct ctl_table *table;
	struct ctl_dir *new;
	struct ctl_node *node;
	char *new_name;

	new = kzalloc(sizeof(*new) + sizeof(struct ctl_node) +
		      sizeof(struct ctl_table)*2 +  namelen + 1,
		      GFP_KERNEL);
	if (!new)
		return NULL;

	node = (struct ctl_node *)(new + 1);
	table = (struct ctl_table *)(node + 1);
	new_name = (char *)(table + 2);
	memcpy(new_name, name, namelen);
	new_name[namelen] = '\0';
	table[0].procname = new_name;
	table[0].mode = S_IFDIR|S_IRUGO|S_IXUGO;
	init_header(&new->header, set->dir.header.root, set, node, table);

	return new;
}


// Source: proc_sysctl.c
// Lines 964-988
