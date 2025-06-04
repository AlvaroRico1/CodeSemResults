int proc_setup_self(struct super_block *s)
{
	struct inode *root_inode = d_inode(s->s_root);
	struct pid_namespace *ns = proc_pid_ns(root_inode);
	struct dentry *self;
	int ret = -ENOMEM;
	
	inode_lock(root_inode);
	self = d_alloc_name(s->s_root, "self");
	if (self) {
		struct inode *inode = new_inode_pseudo(s);
		if (inode) {
			inode->i_ino = self_inum;
			inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
			inode->i_mode = S_IFLNK | S_IRWXUGO;
			inode->i_uid = GLOBAL_ROOT_UID;
			inode->i_gid = GLOBAL_ROOT_GID;
			inode->i_op = &proc_self_inode_operations;
			d_add(self, inode);
			ret = 0;
		} else {
			dput(self);
		}
	}
	inode_unlock(root_inode);

	if (ret)
		pr_err("proc_fill_super: can't allocate /proc/self\n");
	else
		ns->proc_self = self;

	return ret;
}


// Source: self.c
// Lines 36-68
