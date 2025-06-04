static int handle_lookup_down(struct nameidata *nd)
{
	struct path path = nd->path;
	struct inode *inode = nd->inode;
	unsigned seq = nd->seq;
	int err;

	if (nd->flags & LOOKUP_RCU) {
		/*
		 * don't bother with unlazy_walk on failure - we are
		 * at the very beginning of walk, so we lose nothing
		 * if we simply redo everything in non-RCU mode
		 */
		if (unlikely(!__follow_mount_rcu(nd, &path, &inode, &seq)))
			return -ECHILD;
	} else {
		dget(path.dentry);
		err = follow_managed(&path, nd);
		if (unlikely(err < 0))
			return err;
		inode = d_backing_inode(path.dentry);
		seq = 0;
	}
	path_to_nameidata(&path, nd);
	nd->inode = inode;
	nd->seq = seq;
	return 0;
}


// Source: namei.c
// Lines 2274-2301
