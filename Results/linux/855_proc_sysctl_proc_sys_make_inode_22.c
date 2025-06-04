static struct inode *proc_sys_make_inode(struct super_block *sb,
		struct ctl_table_header *head, struct ctl_table *table)
{
	struct ctl_table_root *root = head->root;
	struct inode *inode;
	struct proc_inode *ei;

	inode = new_inode(sb);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	inode->i_ino = get_next_ino();

	ei = PROC_I(inode);

	spin_lock(&sysctl_lock);
	if (unlikely(head->unregistering)) {
		spin_unlock(&sysctl_lock);
		iput(inode);
		return ERR_PTR(-ENOENT);
	}
	ei->sysctl = head;
	ei->sysctl_entry = table;
	hlist_add_head_rcu(&ei->sysctl_inodes, &head->inodes);
	head->count++;
	spin_unlock(&sysctl_lock);

	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	inode->i_mode = table->mode;
	if (!S_ISDIR(table->mode)) {
		inode->i_mode |= S_IFREG;
		inode->i_op = &proc_sys_inode_operations;
		inode->i_fop = &proc_sys_file_operations;
	} else {
		inode->i_mode |= S_IFDIR;
		inode->i_op = &proc_sys_dir_operations;
		inode->i_fop = &proc_sys_dir_file_operations;
		if (is_empty_dir(head))
			make_empty_dir_inode(inode);
	}

	if (root->set_ownership)
		root->set_ownership(head, table, &inode->i_uid, &inode->i_gid);
	else {
		inode->i_uid = GLOBAL_ROOT_UID;
		inode->i_gid = GLOBAL_ROOT_GID;
	}

	return inode;
}


// Source: proc_sysctl.c
// Lines 463-512
