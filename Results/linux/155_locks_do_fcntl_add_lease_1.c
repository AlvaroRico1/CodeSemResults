static int do_fcntl_add_lease(unsigned int fd, struct file *filp, long arg)
{
	struct file_lock *fl;
	struct fasync_struct *new;
	int error;

	fl = lease_alloc(filp, arg);
	if (IS_ERR(fl))
		return PTR_ERR(fl);

	new = fasync_alloc();
	if (!new) {
		locks_free_lock(fl);
		return -ENOMEM;
	}
	new->fa_fd = fd;

	error = vfs_setlease(filp, arg, &fl, (void **)&new);
	if (fl)
		locks_free_lock(fl);
	if (new)
		fasync_free(new);
	return error;
}


// Source: locks.c
// Lines 2020-2043
