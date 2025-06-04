static ssize_t sel_commit_bools_write(struct file *filep,
				      const char __user *buf,
				      size_t count, loff_t *ppos)
{
	struct selinux_fs_info *fsi = file_inode(filep)->i_sb->s_fs_info;
	char *page = NULL;
	ssize_t length;
	int new_value;

	if (count >= PAGE_SIZE)
		return -ENOMEM;

	/* No partial writes. */
	if (*ppos != 0)
		return -EINVAL;

	page = memdup_user_nul(buf, count);
	if (IS_ERR(page))
		return PTR_ERR(page);

	mutex_lock(&fsi->mutex);

	length = avc_has_perm(&selinux_state,
			      current_sid(), SECINITSID_SECURITY,
			      SECCLASS_SECURITY, SECURITY__SETBOOL,
			      NULL);
	if (length)
		goto out;

	length = -EINVAL;
	if (sscanf(page, "%d", &new_value) != 1)
		goto out;

	length = 0;
	if (new_value && fsi->bool_pending_values)
		length = security_set_bools(fsi->state, fsi->bool_num,
					    fsi->bool_pending_values);

	if (!length)
		length = count;

out:
	mutex_unlock(&fsi->mutex);
	kfree(page);
	return length;
}


// Source: selinuxfs.c
// Lines 1266-1311
