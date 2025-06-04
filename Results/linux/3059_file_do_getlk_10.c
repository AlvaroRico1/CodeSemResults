do_getlk(struct file *filp, int cmd, struct file_lock *fl, int is_local)
{
	struct inode *inode = filp->f_mapping->host;
	int status = 0;
	unsigned int saved_type = fl->fl_type;

	/* Try local locking first */
	posix_test_lock(filp, fl);
	if (fl->fl_type != F_UNLCK) {
		/* found a conflict */
		goto out;
	}
	fl->fl_type = saved_type;

	if (NFS_PROTO(inode)->have_delegation(inode, FMODE_READ))
		goto out_noconflict;

	if (is_local)
		goto out_noconflict;

	status = NFS_PROTO(inode)->lock(filp, cmd, fl);
out:
	return status;
out_noconflict:
	fl->fl_type = F_UNLCK;
	goto out;
}


// Source: file.c
// Lines 657-683
