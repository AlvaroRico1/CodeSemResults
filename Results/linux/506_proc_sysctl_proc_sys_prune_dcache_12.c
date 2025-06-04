static void proc_sys_prune_dcache(struct ctl_table_header *head)
{
	struct inode *inode;
	struct proc_inode *ei;
	struct hlist_node *node;
	struct super_block *sb;

	rcu_read_lock();
	for (;;) {
		node = hlist_first_rcu(&head->inodes);
		if (!node)
			break;
		ei = hlist_entry(node, struct proc_inode, sysctl_inodes);
		spin_lock(&sysctl_lock);
		hlist_del_init_rcu(&ei->sysctl_inodes);
		spin_unlock(&sysctl_lock);

		inode = &ei->vfs_inode;
		sb = inode->i_sb;
		if (!atomic_inc_not_zero(&sb->s_active))
			continue;
		inode = igrab(inode);
		rcu_read_unlock();
		if (unlikely(!inode)) {
			deactivate_super(sb);
			rcu_read_lock();
			continue;
		}

		d_prune_aliases(inode);
		iput(inode);
		deactivate_super(sb);

		rcu_read_lock();
	}
	rcu_read_unlock();
}


// Source: proc_sysctl.c
// Lines 270-306
