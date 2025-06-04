static int nfs_can_extend_write(struct file *file, struct page *page, struct inode *inode)
{
	int ret;
	struct file_lock_context *flctx = inode->i_flctx;
	struct file_lock *fl;

	if (file->f_flags & O_DSYNC)
		return 0;
	if (!nfs_write_pageuptodate(page, inode))
		return 0;
	if (NFS_PROTO(inode)->have_delegation(inode, FMODE_WRITE))
		return 1;
	if (!flctx || (list_empty_careful(&flctx->flc_flock) &&
		       list_empty_careful(&flctx->flc_posix)))
		return 1;

	/* Check to see if there are whole file write locks */
	ret = 0;
	spin_lock(&flctx->flc_lock);
	if (!list_empty(&flctx->flc_posix)) {
		fl = list_first_entry(&flctx->flc_posix, struct file_lock,
					fl_list);
		if (is_whole_file_wrlock(fl))
			ret = 1;
	} else if (!list_empty(&flctx->flc_flock)) {
		fl = list_first_entry(&flctx->flc_flock, struct file_lock,
					fl_list);
		if (fl->fl_type == F_WRLCK)
			ret = 1;
	}
	spin_unlock(&flctx->flc_lock);
	return ret;
}


// Source: write.c
// Lines 1313-1345
