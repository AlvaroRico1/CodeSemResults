static struct proc_dir_entry *pde_subdir_find(struct proc_dir_entry *dir,
					      const char *name,
					      unsigned int len)
{
	struct rb_node *node = dir->subdir.rb_node;

	while (node) {
		struct proc_dir_entry *de = rb_entry(node,
						     struct proc_dir_entry,
						     subdir_node);
		int result = proc_match(name, de, len);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return de;
	}
	return NULL;
}


// Source: generic.c
// Lines 68-88
