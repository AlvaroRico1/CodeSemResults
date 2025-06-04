int nfs_flock(struct file *filp, int cmd, struct file_lock *fl)
{
	struct inode *inode = filp->f_mapping->host;
	int is_local = 0;

	dprintk("NFS: flock(%pD2, t=%x, fl=%x)\n",
			filp, fl->fl_type, fl->fl_flags);

	if (!(fl->fl_flags & FL_FLOCK))
		return -ENOLCK;

	/*
	 * The NFSv4 protocol doesn't support LOCK_MAND, which is not part of
	 * any standard. In principle we might be able to support LOCK_MAND
	 * on NFSv2/3 since NLMv3/4 support DOS share modes, but for now the
	 * NFS code is not set up for it.
	 */
	if (fl->fl_type & LOCK_MAND)
		return -EINVAL;

	if (NFS_SERVER(inode)->flags & NFS_MOUNT_LOCAL_FLOCK)
		is_local = 1;

	/* We're simulating flock() locks using posix locks on the server */
	if (fl->fl_type == F_UNLCK)
		return do_unlk(filp, cmd, fl, is_local);
	return do_setlk(filp, cmd, fl, is_local);
}


// Source: file.c
// Lines 805-832
