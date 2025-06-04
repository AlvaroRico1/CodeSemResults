static void remove_files(struct kernfs_node *parent,
			 const struct attribute_group *grp)
{
	struct attribute *const *attr;
	struct bin_attribute *const *bin_attr;

	if (grp->attrs)
		for (attr = grp->attrs; *attr; attr++)
			kernfs_remove_by_name(parent, (*attr)->name);
	if (grp->bin_attrs)
		for (bin_attr = grp->bin_attrs; *bin_attr; bin_attr++)
			kernfs_remove_by_name(parent, (*bin_attr)->attr.name);
}


// Source: group.c
// Lines 19-31
